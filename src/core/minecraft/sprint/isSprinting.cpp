#include <Windows.h>

#include "../minecraft.h"
#include "../../methods/methods.h"
#include "../../jvm.h"
#include "../../classes/classes.h"

// checks if the entity is currently sprinting.
bool cEntity::isSprinting() {
  JNIEnv *env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.entity);
  if (!env || !cls || !cachedObject) return false;

  jmethodID method = jvm::methodId(cls, "method_5624", "()Z");
  if (!method) return false;

  jboolean is = env->CallBooleanMethod(cachedObject, method);
  jvm::checkException("isSprinting");

  return is == JNI_TRUE;
}