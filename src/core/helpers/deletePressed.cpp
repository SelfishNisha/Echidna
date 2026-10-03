#include "helpers.h"

// check if the unload key is held down.
bool deletePressed() {
  return (GetAsyncKeyState(VK_DELETE) & 0x8000) != 0;
}
