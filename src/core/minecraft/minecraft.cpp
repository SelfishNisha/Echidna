#include "minecraft.h"
#include "../fields/fields.h"
#include "../classes/classes.h"

// Fetches the game's singleton Minecraft instance from the JVM.
// Returns a wrapper around null if anything can't be resolved.
cMinecraft cMinecraft::getMinecraft() {
  JNIEnv* env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.minecraft);

  if (!env || !cls) return cMinecraft(nullptr);

  // class_310 is the obfuscated (intermediary) name for the Minecraft class.
  // The last argument marks the field as static.
  jfieldID field = jvm::fieldId(cls, fields.minecraft, "Lnet/minecraft/class_310;", true);

  if (!field) return cMinecraft(nullptr);

  return cMinecraft(env->GetStaticObjectField(cls, field));
}

// Gets the local player from the Minecraft instance.
// Returns a wrapper around null if the instance or field isn't available.
cEntity cMinecraft::getPlayer() const {
  JNIEnv *env = jvm::getEnv();
  jclass cls = jvm::lookFor(classes.minecraft);

  if (!env || !cls || !cachedObject) return cEntity(nullptr);

  // class_746 is the obfuscated name for the client player class.
  jfieldID field = jvm::fieldId(cls, fields.player, "Lnet/minecraft/class_746;");
  
  if (!field) return cEntity(nullptr);

  return cEntity(env->GetObjectField(cachedObject, field)); // null in the main menu, which is fine
}