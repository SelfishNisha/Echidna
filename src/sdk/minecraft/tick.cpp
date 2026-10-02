#include <iostream>

#include "../jvm.h"
#include "minecraft.h"

void tick() {
  auto mc = cMinecraft::getMinecraft();
  std::cout << "Minecraft instance: " << mc.chachedObject << std::endl;

  if (mc.chachedObject) {
    auto player = mc.getPlayer();
    std::cout << "Player: " << player.chachedObject << std::endl;
  }
}