#include "jvm.h"
#include "../core/helper/helpers.h"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <unordered_map>

namespace {
  JavaVM *gVm = nullptr;
  jobject gLoader = nullptr;
  jmethodID gLoadClass = nullptr;

  std::atomic<bool> gReady = false;
  std::atomic<bool> gShutdown = false;

  std::mutex gChachedMutex;
  std::unordered_map<std::string, jclass> gClasses;
  std::unordered_map<std::string, void *> gIds;

  std::mutex gErrorMutex;
  std::string gLastError;

  thread_local JNIEnv *tlEnv = nullptr;
  thread_local bool tlAttached = false;

  void setError(const std::string &msg) {
    std::lock_guard<std::mutex> lock(gErrorMutex);
    gLastError = msg;
    std::cout << "[ERROR] " << msg << std::endl;
  }

  std::string jstr(JNIEnv *env, jstring str) {
    if (!str) return "<null>";

    const char *u = env->GetStringUTFChars(str, nullptr);
    if (!u) return "<?>";

    env->ReleaseStringUTFChars(str, u);
    return std::string(u);
  }

  std::string describeException(JNIEnv *env, jthrowable exception) {
    std::string out = "<unknown>";

    if (!exception) return out;

    jclass obj = env->FindClass("java/lang/Object");
    jmethodID toString = obj ? env->GetMethodID(obj, "toString", "()Ljava/lang/String;") : nullptr;

    if (toString) {
      auto str = static_cast<jstring>(env->CallObjectMethod(exception, toString));

      if (env->ExceptionCheck()) {
        env->ExceptionClear();
      } else if (str) {
        out = jstr(env, str);
        env->DeleteLocalRef(str);
      }
    }

    if (obj) {
      env->DeleteLocalRef(obj);
    }
    return out;
  }

  // Finds a thread context class loader that can really load the probe class.
  jobject findGameLoader(JNIEnv *env, const std::string &probeDotted) {
    jobject result = nullptr;
    if (env->PushLocalFrame(64) != JNI_OK) return nullptr;

    do {
      jclass threadClass = env->FindClass("java/lang/Thread");
      jclass mapClass = env->FindClass("java/util/Map");
      jclass setClass = env->FindClass("java/util/Set");
      jclass clClass = env->FindClass("java/lang/ClassLoader");

      if (!threadClass || !mapClass || !setClass || !clClass) break;

      jmethodID getAll = env->GetStaticMethodID(threadClass, "getAllStackTraces", "()Ljava/util/Map;");
      jmethodID getName = env->GetMethodID(threadClass, "getName", "()Ljava/lang/String;");
      jmethodID getCll = env->GetMethodID(threadClass, "getContextClassLoader", "()Ljava/lang/ClassLoader;");
      jmethodID keySet = env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;");
      jmethodID toArray = env->GetMethodID(setClass, "toArray", "()[Ljava/lang/Object;");
      jmethodID loadClass = env->GetMethodID(clClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

      if (env->ExceptionCheck() || !getAll || !getName || !getCll || !keySet || !toArray || !loadClass) break;

      jobject map = env->CallStaticObjectMethod(threadClass, getAll);
      if (env->ExceptionCheck() || !map) break;

      jobject keys = env->CallObjectMethod(map, keySet);
      if (env->ExceptionCheck() || !keys) break;

      auto threads = static_cast<jobjectArray>(env->CallObjectMethod(keys, toArray));
      if (env->ExceptionCheck() || !threads) break;

      jstring probeStr = env->NewStringUTF(probeDotted.c_str());
      jsize count = env->GetArrayLength(threads);

      for (jsize i = 0; i < count && !result; i++) {
        jobject thread = env->GetObjectArrayElement(threads, i);

        if (!thread) continue;

        jobject loader = env->CallObjectMethod(thread, getCll);

        if (env->ExceptionCheck()) {
          env->ExceptionClear();
          loader = nullptr;
        }

        if (loader) {
          jobject cls = env->CallObjectMethod(loader, loadClass, probeStr);

          if (env->ExceptionCheck()) {
            env->ExceptionClear();
            cls = nullptr;
          }

          if (cls) {
            auto name = static_cast<jstring>(env->CallObjectMethod(thread, getName));
            
            if (env->ExceptionCheck()) {
              env->ExceptionClear();
              name = nullptr;
            }

            std::cout << "[Echidna] Class loader taken from thread: " << (name ? jstr(env, name) : "<?>") << std::endl;

            result = env->NewGlobalRef(loader);

            gLoadClass = loadClass; // ClassLoader.loadClass works for every subclass

            if (name) env->DeleteLocalRef(name);
            env->DeleteLocalRef(cls);
          }

          env->DeleteLocalRef(loader);
        }

        env->DeleteLocalRef(thread);
      }
    } while (false);

    if (env->ExceptionCheck()) env->ExceptionClear();
    env->PopLocalFrame(nullptr);
    return result;
  }

  std::string makeKey(char kind, jclass cls, const char *name, const char *sig, bool isStatic) {
    return std::string(1, kind) + (isStatic ? "s|" : "i|") + std::to_string(reinterpret_cast<std::uintptr_t>(cls)) + "|" + name + "|" + sig;
  }
}

// Lifecycle

JNIEnv *jvm::getEnv() {
  if (gShutdown.load() || !gVm) return nullptr;
  if (tlEnv) return tlEnv;

  JNIEnv *env = nullptr;
  jint status = gVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_8);

  if (status == JNI_EDETACHED) {
    if (gVm->AttachCurrentThreadAsDaemon(reinterpret_cast<void **>(&env), nullptr) != JNI_OK) {
      setError("Failed to attach thread to JVM");
      return nullptr;
    }

    tlAttached = true;
  } else if (status != JNI_OK) {
    setError("getEnv failed");
    return nullptr;
  }

  tlEnv = env;
  return env;
}

void jvm::detachCurrentThread() {
  if (tlAttached && gVm) {
    gVm->DetachCurrentThread();
  }
  tlAttached = false;
  tlEnv = nullptr;
}

bool jvm::init(const char *probeClass) {
  if (gReady.load()) return true;

  JavaVM *vm = nullptr;
  jsize count = 0;
  
  if (JNI_GetCreatedJavaVMs(&vm, 1, &count) != JNI_OK || count < 1 || !vm) {
    setError("No java VM found");
    return false;
  }

  gVm = vm;
  JNIEnv *env = getEnv();

  if (!env) return false;

  gLoader = findGameLoader(env, toDotted(probeClass));

  if (!gLoader) {
    setError("Failed to find game class loader, falling back to FindClass");
  }

  gReady.store(true);
  return true;
}

bool jvm::isReady() {
  return gReady.load() && !gShutdown.load();
}

void jvm::shutdown() {
  JNIEnv *env = getEnv();

  if (gShutdown.exchange(true)) return;

  gReady.store(false);

  if (env) {
    std::lock_guard<std::mutex> lock(gChachedMutex);

    for (auto &entry : gClasses) {
      env->DeleteGlobalRef(entry.second);
    }

    gClasses.clear();
    gIds.clear();

    if (gLoader) {
      env->DeleteGlobalRef(gLoader);
    }

    if (env->ExceptionCheck()) {
      env->ExceptionClear();
    }
  }

  gLoader = nullptr;
  gLoadClass = nullptr;

  if (tlAttached && gVm) {
    gVm->DetachCurrentThread();
  }

  tlAttached = false;
  tlEnv = nullptr;
  gVm = nullptr;
}

std::string jvm::lastError() {
  std::lock_guard<std::mutex> lock(gErrorMutex);
  return gLastError;
}

// Exceptions

bool jvm::checkException(const std::string &where) {
  JNIEnv *env = getEnv();

  if (!env || !env->ExceptionCheck()) return false;

  jthrowable exception = env->ExceptionOccurred();
  env->ExceptionClear();

  std::string text = describeException(env, exception);

  if (exception) {
    env->DeleteLocalRef(exception);
  }

  setError((where.empty() ? "" : where + ": ") + text);
  return true;
}

// Lookups

jclass jvm::lookFor(const std::string &name) {
  JNIEnv *env = getEnv();

  if (!env) return nullptr;

  const std::string dotted = toDotted(name);

  {
    std::lock_guard<std::mutex> lock(gChachedMutex);
    auto it = gClasses.find(dotted);
    if (it != gClasses.end()) return it->second;
  }

  jclass found = nullptr;

  // the game class loader
  if (gLoader && gLoadClass) {
    jstring jname = env->NewStringUTF(dotted.c_str());
    jobject local = env->CallObjectMethod(gLoader, gLoadClass, jname);
    env->DeleteLocalRef(jname);

    if (!checkException("loadClass(" + dotted + ")") && local) found = static_cast<jclass>(env->NewGlobalRef(local));
    if (local) env->DeleteLocalRef(local);
  }

  // plain FindClass
  if (!found) {
    jclass local = env->FindClass(toSlashed(dotted).c_str());
    
    if (!checkException("FindClass(" + dotted + ")") && local) found = static_cast<jclass>(env->NewGlobalRef(local));
    if (local) env->DeleteLocalRef(local);
  }

  if (!found) {
    setError("class not found: " + dotted);
    return nullptr; // failures are not cached, so we can retry later
  }

  std::lock_guard<std::mutex> lock(gChachedMutex);
  auto result = gClasses.emplace(dotted, found);

  if (!result.second) {
    // another thread got there first
    env->DeleteGlobalRef(found);
    found = result.first->second;
  }
  return found;
}

jfieldID jvm::fieldId(jclass cls, const char *name, const char *sig, bool isStatic) {
  JNIEnv *env = getEnv();

  if (!env || !cls) return nullptr;

  const std::string key = makeKey('F', cls, name, sig, isStatic);

  {
    std::lock_guard<std::mutex> lock(gChachedMutex);
    auto it = gIds.find(key);
    if (it != gIds.end()) return static_cast<jfieldID>(it->second);
  }

  jfieldID id = isStatic ? env->GetStaticFieldID(cls, name, sig) : env->GetFieldID(cls, name, sig);

  if (checkException(std::string("field ") + name) || !id) return nullptr;

  std::lock_guard<std::mutex> lock(gChachedMutex);
  gIds[key] = static_cast<void *>(id);
  return id;
}

jmethodID jvm::methodId(jclass cls, const char *name, const char *sig, bool isStatic) {
  JNIEnv *env = getEnv();

  if (!env || !cls) return nullptr;

  const std::string key = makeKey('M', cls, name, sig, isStatic);

  {
    std::lock_guard<std::mutex> lock(gChachedMutex);
    auto it = gIds.find(key);
    if (it != gIds.end()) return static_cast<jmethodID>(it->second);
  }

  jmethodID id = isStatic ? env->GetStaticMethodID(cls, name, sig) : env->GetMethodID(cls, name, sig);

  if (checkException(std::string("method ") + name) || !id) return nullptr;

  std::lock_guard<std::mutex> lock(gChachedMutex);
  gIds[key] = static_cast<void *>(id);
  return id;
}

// cJObject

cJObject::cJObject(jobject localRef) {
  if (!localRef) return;

  JNIEnv *env = jvm::getEnv();
  if (!env) return;

  chachedObject = env->NewGlobalRef(localRef);
  env->DeleteLocalRef(localRef);
}

cJObject::~cJObject() {
  if (!chachedObject) return;

  // after shutdown the ref is simply leaked, which is harmless
  if (JNIEnv *env = jvm::getEnv()) env->DeleteGlobalRef(chachedObject);
}

cJObject::cJObject(cJObject &&other) noexcept : chachedObject(other.chachedObject) {
  other.chachedObject = nullptr;
}

cJObject &cJObject::operator=(cJObject &&other) noexcept {
  if (this != &other) {
    if (chachedObject) {
      if (JNIEnv *env = jvm::getEnv()) {
        env->DeleteGlobalRef(chachedObject);
      }
      chachedObject = other.chachedObject;
      other.chachedObject = nullptr;
    }
  }
  return *this;
}