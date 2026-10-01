#include "minecraft.h"
#include "../classes/classes.h"
#include "../jvm.h"

c_minecraft::c_minecraft(jobject object_in) : c_jobject(object_in) {

}

c_minecraft c_minecraft::get_minecraft() {
  static jfieldID instance_field = jvm::env->GetStaticFieldID(
      classes::minecraft_class, "field_1700", "Lnet/minecraft/class_310;");

  return c_minecraft(jvm::env->GetStaticObjectField(classes::minecraft_class, instance_field));
}