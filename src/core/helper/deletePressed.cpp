#include "helpers.h"

bool deletePressed() {
  return (GetAsyncKeyState(VK_DELETE) & 0x8000) != 0;
}
