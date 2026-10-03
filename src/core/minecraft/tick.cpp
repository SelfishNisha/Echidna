#include <iostream>

#include "../jvm.h"
#include "minecraft.h"

void tick() {
  auto mc = cMinecraft::getMinecraft();
  if (mc.chachedObject) {
    auto player = mc.getPlayer();

    player.setSprinting(true);
  }
}