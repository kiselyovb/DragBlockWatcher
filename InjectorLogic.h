#pragma once

#include <string>

// Проверка: является ли процесс GUI (содержит user32.dll)
bool IsGuiProcess(DWORD pid);

// Выполнение инъекции DLL в указанный процесс
bool InjectDLL(DWORD pid, const std::wstring& dllPath);

// (Опционально) Проверка, внедрена ли DLL уже
bool IsAlreadyInjected(DWORD pid, const std::wstring& dllName);
