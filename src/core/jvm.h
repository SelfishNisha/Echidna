#pragma once
#include <jni/jni.h>
#include <string>

namespace jvm {
  bool init(const char *probeClass); // attach + find the game class loader
  void shutdown(); // frees everything, detaches calling thread
  bool isReady();

  JNIEnv* getEnv(); // per-thread env, auto-attaches. null if unavailable
  void detachCurrentThread(); // detaches the calling thread from the jvm, if attached

  // class lookup accepts "a.b.C" or "a/b/C", cached, returns a global ref 
  jclass lookFor(const std::string& name);

  // cached id lookups
  jfieldID fieldId(jclass cls, const char *name, const char *sig, bool isStatic = false);
  jmethodID methodId(jclass cls, const char *name, const char *sig, bool isStatic = false);

  // returns true if a Java exception was pending (logs and clears it)
  bool checkException(const std::string& where = "");

  std::string lastError();
}

// Owns a GLOBAL ref, so it is safe to create on one thread and use/destroy on another.
class cJObject {
  public:
    cJObject(jobject localRef); // takes ownership of the local ref and promotes it
    ~cJObject();

    cJObject(const cJObject&) = delete;
    cJObject& operator=(const cJObject&) = delete;
    cJObject(cJObject&& other) noexcept;
    cJObject& operator=(cJObject&& other) noexcept;

    jobject chachedObject = nullptr; // the global ref, or null if none
};