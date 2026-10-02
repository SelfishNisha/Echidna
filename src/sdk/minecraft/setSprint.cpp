#include <Windows.h>
#include <iostream>

#include "minecraft.h"
#include "../jvm.h"
#include "../classes/classes.h"

void cEntity::setSprinting(bool state) {
  JNIEnv *env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.entity);
  if (!env || !cls || !chachedObject) return;

  jmethodID method = jvm::methodId(cls, "method_5728", "(Z)V");
  if (!method) return;

  // if ((GetAsyncKeyState('W') & 0x8000) == 0) return;

  env->CallVoidMethod(chachedObject, method, static_cast<jboolean>(state));
  jvm::checkException("setSprinting");

  std::cout << "[Echidna] Sprint Set To True" << std::endl;
}