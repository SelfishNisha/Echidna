#include "jvm.h"
#include <cstring>
#include <string>

// find game class loader
static jobject fgcl() {
  JNIEnv *env = jvm::env;

  std::string classes[] = {
    "java/lang/Thread",
    "java/util/Map",
    "java/util/Set"
  };

  std::string methods[] = {
    "getAllStackTraces",
    "getName",
    "getContextClassLoader",
    "keySet",
    "toArray"
  };

  std::string signatures[] = {
    "()Ljava/util/Map;",
    "()Ljava/lang/String;",
    "()Ljava/lang/ClassLoader;",
    "()Ljava/util/Set;",
    "()[Ljava/lang/Object;"
  };

  auto threads = static_cast<jobjectArray>(env->CallStaticObjectMethod(
      env->FindClass(classes[0].c_str()),
      env->GetStaticMethodID(env->FindClass(classes[0].c_str()), methods[0].c_str(), signatures[0].c_str())));

  jobject result = nullptr;
  jsize thread_count = env->GetArrayLength(threads);

  for (jsize i = 0; i < thread_count && !result; i++) {
    jobject thread = env->GetObjectArrayElement(threads, i);
    auto name = static_cast<jstring>(env->CallObjectMethod(thread, env->GetMethodID(env->FindClass(classes[0].c_str()), methods[1].c_str(), signatures[1].c_str())));
    const char *utf = env->GetStringUTFChars(name, nullptr);

    if (std::strcmp(utf, "Render thread") == 0) {
      jobject loader = env->CallObjectMethod(thread, env->GetMethodID(env->FindClass(classes[0].c_str()), methods[2].c_str(), signatures[2].c_str()));
      result = env->NewGlobalRef(loader);
      env->DeleteLocalRef(loader);
    }

    env->ReleaseStringUTFChars(name, utf);
    env->DeleteLocalRef(name);
    env->DeleteLocalRef(thread);
  }
  env->DeleteLocalRef(threads);
  return result;
}

void jvm::load() {
  JNI_GetCreatedJavaVMs(&vm, 1, nullptr);

  jint status = vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_8);
  if (status == JNI_EDETACHED) {
    vm->AttachCurrentThreadAsDaemon(reinterpret_cast<void **>(&env), nullptr);
  }

  class_loader = fgcl();
}

jclass jvm::find_class(const char *dotted_name) {
  jclass loader_cls = env->GetObjectClass(class_loader);
  jmethodID load_class = env->GetMethodID(loader_cls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

  jstring name = env->NewStringUTF(dotted_name);
  jclass cls = static_cast<jclass>(env->CallObjectMethod(class_loader, load_class, name));
  env->DeleteLocalRef(name);

  auto global = static_cast<jclass>(env->NewGlobalRef(cls)); // keep it valid across calls
  env->DeleteLocalRef(cls);
  return global;
}

c_jobject::c_jobject(jobject object_in) {
  this->cached_object = object_in;
}

c_jobject::~c_jobject() {
  if (this->cached_object != nullptr) {
    jvm::env->DeleteLocalRef(this->cached_object);
  }
}