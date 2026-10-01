#include "classes.h"
#include "../jvm.h"

void classes::load() {
  classes::minecraft_class = jvm::find_class("net.minecraft.class_310");
}