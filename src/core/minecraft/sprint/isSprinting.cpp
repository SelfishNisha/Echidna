#include <Windows.h>

#include "../minecraft.h"
#include "../../methods/methods.h"
#include "../../jvm.h"
#include "../../classes/classes.h"

bool cEntity::isSprinting() {
  JNIEnv *env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.entity);
  if (!env || !cls || !chachedObject) return false;

  jmethodID method = jvm::methodId(cls, "method_5624", "()Z");
  if (!method) return false;

  jboolean is = env->CallBooleanMethod(chachedObject, method);
  jvm::checkException("isSprinting");

  return is == JNI_TRUE;
}