#pragma once
#include <jni/jni.h>

namespace jvm {
  void load();
  jclass find_class(const char* dotted_name); // e.g. "net.minecraft.class_310"

  inline JavaVM* vm = nullptr;
  inline JNIEnv* env = nullptr;
  inline jobject class_loader = nullptr; // global ref
}

class c_jobject {
  public:
   c_jobject(jobject object_in);
   ~c_jobject();

   c_jobject(const c_jobject&) = delete;
   c_jobject& operator=(const c_jobject&) = delete;

   c_jobject(c_jobject&& other) noexcept : cached_object(other.cached_object) {
     other.cached_object = nullptr;
   }

   jobject cached_object = nullptr;
};