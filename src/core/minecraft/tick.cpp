#include "../jvm.h"
#include "minecraft.h"
#include "../helpers/helpers.h"
#include "../modules/modules.h"

void tick() {
  auto mc = cMinecraft::getMinecraft();
  if (!mc.cachedObject) return; // wtf :|

  autoSprint(mc);
}