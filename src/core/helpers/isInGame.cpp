#include <iostream>

#include "helpers.h"

// returns 1 (true) if the user in inside the game.
bool isInGame() {
  DWORD pid = 0;
  GetWindowThreadProcessId(GetForegroundWindow(), &pid);
  return pid == GetCurrentProcessId();
}