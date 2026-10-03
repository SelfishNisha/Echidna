#include "helpers.h"

// Removes the close button from the console window, since closing it
// would kill the whole host process (the game) along with it.
void disableConsoleClose() {
  if (HWND console = GetConsoleWindow()) {
    // Deleting SC_CLOSE from the system menu also greys out the X button.
    if (HMENU menu = GetSystemMenu(console, FALSE)) DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
  }
}