#include <iostream>

#include "../jvm.h"
#include "minecraft.h"
#include "../helpers/helpers.h"

void tick() {
  auto mc = cMinecraft::getMinecraft();
  if (!mc.cachedObject || !isInGame()) return;

  // auto player = 

  isInGame();
}