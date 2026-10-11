#include "minecraft.h"
#include "../jvm.h"
#include "../classes/classes.h"
#include "../fields/fields.h"
#include "../methods/methods.h"

cGameOptions cMinecraft::getOptions() {
  JNIEnv* env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.minecraft);
  if (!env || !cls || !cachedObject) return cGameOptions(nullptr);

  jfieldID field = jvm::fieldId(cls, "field_1690", "Lnet/minecraft/class_315;");
  if (!field) return cGameOptions(nullptr);

  return cGameOptions(env->GetObjectField(cachedObject, field));
}

cKeyBinding cGameOptions::getSprintKey() {
  JNIEnv* env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.gameOptions);
  if (!env || !cls || !cachedObject) return cKeyBinding(nullptr);

  jfieldID field = jvm::fieldId(cls, fields.gameOptions_sprintKey, "Lnet/minecraft/class_304;");
  if (!field) return cKeyBinding(nullptr);

  return cKeyBinding(env->GetObjectField(cachedObject, field));
}

void cKeyBinding::setPressed(bool pressed) {
  JNIEnv* env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.keyBinding);
  if (!env || !cls || !cachedObject)
    return;

  jmethodID method = jvm::methodId(cls, methods.keys_setPressed, "(Z)V");
  if (!method) return;

  env->CallVoidMethod(cachedObject, method, static_cast<jboolean>(pressed));
  jvm::checkException("KeyBinding.setPressed");
}