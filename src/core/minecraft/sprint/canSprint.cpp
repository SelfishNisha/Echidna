#include <Windows.h>

#include "../minecraft.h"
#include "../../methods/methods.h"
#include "../../jvm.h"
#include "../../classes/classes.h"

bool cEntity::canSprint() {
  JNIEnv *env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.entity);
  if (!env || !cls || !chachedObject) return false;

  jmethodID method = jvm::methodId(cls, "method_48155", "()Z");
  if (!method) return false;

  jboolean can = env->CallBooleanMethod(chachedObject, method);
  jvm::checkException("canSprint");

  return can == JNI_TRUE;
}