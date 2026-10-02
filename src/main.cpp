#include <Windows.h>
#include <jni/jni.h>
#include <iostream>
#include <exception>

#include "core/helper/helpers.h"

#include "jvm.h"
#include "classes/classes.h"
#include "minecraft/minecraft.h"

// the codes that will be executed when the dll is injected.
DWORD WINAPI threadEntry(LPVOID param) {
  auto module = static_cast<HMODULE>(param);
  FILE* outputBuffer = nullptr;

  AllocConsole();
  disableConsoleClose();
  freopen_s(&outputBuffer, "CONOUT$", "w", stdout);

  std::cout << "[Echidna] Started" << std::endl;

  // Checking if echidna was injected properly.
  if (jvm::init(classes.minecraft) && jvm::lookFor(classes.minecraft)) {
    std::cout << "[Echidna] Ready, press DELETE to unload" << std::endl;

    while (!deletePressed()) {
      try {
        tick();
      } catch (const std::exception& error) {
        std::cout << "[ERROR] " << error.what() << std::endl;
      } catch (...) {
        std::cout << "[ERROR] Unknown error occurred" << std::endl;
      }

      Sleep(10); // will be changed later.
    }
  } else {
    std::cout << "[Echidna] Setup failed: " << jvm::lastError() << std::endl;
    std::cout << "[Echidna] Press DELETE to unload" << std::endl;
    Sleep(300);
    while (!deletePressed()) Sleep(50);
  }

  jvm::shutdown(); // frees all global refs and detaches this thread

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