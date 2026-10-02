#include "helpers.h"

void disableConsoleClose() {
  if (HWND console = GetConsoleWindow()) {
    if (HMENU menu = GetSystemMenu(console, FALSE)) DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
  }
}