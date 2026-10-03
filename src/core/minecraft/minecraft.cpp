#include "minecraft.h"
#include "../fields/fields.h"
#include "../classes/classes.h"

cMinecraft cMinecraft::getMinecraft() {
  JNIEnv* env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.minecraft);

  if (!env || !cls) return cMinecraft(nullptr);

  jfieldID field = jvm::fieldId(cls, fields.minecraft, "Lnet/minecraft/class_310;", true);

  if (!field) return cMinecraft(nullptr);

  return cMinecraft(env->GetStaticObjectField(cls, field));
}

cEntity cMinecraft::getPlayer() const {
  JNIEnv *env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.minecraft);

  if (!env || !cls || !chachedObject) return cEntity(nullptr);

  jfieldID field = jvm::fieldId(cls, fields.player, "Lnet/minecraft/class_746;");
  
  if (!field) return cEntity(nullptr);

  return cEntity(env->GetObjectField(chachedObject, field)); // null in the main menu, which is fine
}