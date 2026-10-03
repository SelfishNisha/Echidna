#include <Windows.h>
#include <iostream>

#include "../minecraft.h"
#include "../../methods/methods.h"
#include "../../jvm.h"
#include "../../classes/classes.h"

// sets the entity's sprinting state to the given value (true or false)
void cEntity::setSprinting(bool state) {
  JNIEnv *env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.entity);
  if (!env || !cls || !cachedObject) return;

  jmethodID method = jvm::methodId(cls, methods.eSetSprinting, "(Z)V");
  if (!method) return;

  env->CallVoidMethod(cachedObject, method, static_cast<jboolean>(state));
  jvm::checkException("setSprinting");

  std::cout << "[Echidna] Sprint Set To " << (state ? "True" : "False") << std::endl;
}