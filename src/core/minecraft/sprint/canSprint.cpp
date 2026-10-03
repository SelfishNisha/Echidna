#include <Windows.h>

#include "../minecraft.h"
#include "../../methods/methods.h"
#include "../../jvm.h"
#include "../../classes/classes.h"

// checks if the entity can sprint.
bool cEntity::canSprint() {
  JNIEnv *env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.entity);
  if (!env || !cls || !cachedObject) return false;

  jmethodID method = jvm::methodId(cls, "method_48155", "()Z");
  if (!method) return false;

  jboolean can = env->CallBooleanMethod(cachedObject, method);
  jvm::checkException("canSprint");

  return can == JNI_TRUE;
}