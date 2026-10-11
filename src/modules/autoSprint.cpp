#include "modules.h"

void autoSprint(cMinecraft& mc) {
  static bool holdingSprint = false;

  if (!mc.cachedObject) return; // wtf :|

  auto player = mc.getPlayer();
  if (!player.cachedObject) { holdingSprint = false; return; } // not in a world

  auto sprintKey = mc.getOptions().getSprintKey();
  if (!sprintKey.cachedObject) return; // wtf :|

  bool holdingForward = isInGame() && (GetAsyncKeyState('W') & 0x8000) != 0; // cute little bug... i'll fix it if anybody ran into it

  if (holdingForward) {
    sprintKey.setPressed(true);
    holdingSprint = true;
  } else if (holdingSprint) {
    sprintKey.setPressed(false);
    holdingSprint = false;
  }
}