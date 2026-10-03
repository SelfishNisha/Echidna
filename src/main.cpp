#include <Windows.h>
#include <jni/jni.h>
#include <iostream>
#include <exception>

#include "core/helpers/helpers.h"

#include "jvm.h"
#include "classes/classes.h"
#include "minecraft/minecraft.h"

// the codes that will be executed when the dll is injected.
DWORD WINAPI threadEntry(LPVOID param) {
  // DllMain hands us the module handle so the DLL can unload itself at the end.
  auto module = static_cast<HMODULE>(param);
  FILE* outputBuffer = nullptr;

  // The host process has no console, so create one and point stdout at it.
  // Closing it is disabled because that would kill the whole host process.
  AllocConsole();
  disableConsoleClose();
  freopen_s(&outputBuffer, "CONOUT$", "w", stdout);

  std::cout << "[Echidna] Started" << std::endl;

  // Checking if echidna was injected properly.
  if (jvm::init(classes.minecraft) && jvm::lookFor(classes.minecraft)) {
    std::cout << "[Echidna] Ready, press DELETE to unload" << std::endl;

    // Main loop, runs until DELETE is pressed.
    while (!deletePressed()) {
      // Errors in a tick are logged instead of crashing the host process.
      try {
        tick();
      } catch (const std::exception& error) {
        std::cout << "[ERROR] " << error.what() << std::endl;
      } catch (...) {
        std::cout << "[ERROR] Unknown error occurred" << std::endl;
      }

      Sleep(1000); // will be changed later.
    }
  } else {
    // Setup failed: show why, then idle until the user chooses to unload.
    std::cout << "[Echidna] Setup failed: " << jvm::lastError() << std::endl;
    std::cout << "[Echidna] Press DELETE to unload" << std::endl;
    Sleep(300);
    while (!deletePressed()) Sleep(50);
  }

  jvm::shutdown(); // frees all global refs and detaches this thread

  // Cleanup, then unload the DLL. FreeLibraryAndExitThread is used because
  // a plain FreeLibrary + return would run code from an already-unloaded DLL.
  if (outputBuffer) fclose(outputBuffer);
  FreeConsole();
  FreeLibraryAndExitThread(module, 0ul);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
  (void)reserved;

  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(module);
    HANDLE thread = CreateThread(nullptr, 0ul, threadEntry, module, 0ul, nullptr);
    if (thread && thread != INVALID_HANDLE_VALUE) CloseHandle(thread);
  }

  return TRUE;
}