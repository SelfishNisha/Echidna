#include <Windows.h>
#include <jni/jni.h>
#include <iostream>

#include "jvm.h"
#include "classes/classes.h"
#include "minecraft/minecraft.h"

DWORD WINAPI thread_entry(LPVOID param) {
  auto module = static_cast<HMODULE>(param);
  FILE* output_buffer = nullptr;
  AllocConsole();

  freopen_s(&output_buffer, "CONOUT$", "w", stdout);

  jvm::load();
  classes::load();

  while (!GetAsyncKeyState(VK_DELETE)) {
    std::cout << "Minecraft class: " << classes::minecraft_class << std::endl;
    std::cout << "Minecraft instance: " << c_minecraft::get_minecraft().cached_object << std::endl;

    Sleep(1000);
  }

  if (output_buffer) {
    fclose(output_buffer);
    output_buffer = nullptr;
  }

  FreeConsole();
  FreeLibraryAndExitThread(module, 0ul);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
  (void)reserved;

  if (reason == DLL_PROCESS_ATTACH) {
    HANDLE thread = CreateThread(nullptr, 0ul, thread_entry, module, 0ul, nullptr);

    if (thread && thread != INVALID_HANDLE_VALUE) {
      CloseHandle(thread);
    }
  }

  return TRUE;
}