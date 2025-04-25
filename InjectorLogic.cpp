/*
Модули InjectorLogic содержит логику, отвечающую за :
Инъекция DLL в процесс по PID.
Определение, является ли процесс GUI - приложением.
Проверка, внедрена ли уже DLL.
*/
#include "pch.h"

#include "InjectorLogic.h"
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

#pragma comment(lib, "Psapi.lib")

bool IsGuiProcess(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return false;

    HMODULE hMods[1024];
    DWORD cbNeeded;
    bool found = false;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            wchar_t szModName[MAX_PATH];
            if (GetModuleBaseNameW(hProcess, hMods[i], szModName, MAX_PATH)) {
                if (_wcsicmp(szModName, L"user32.dll") == 0) {
                    found = true;
                    break;
                }
            }
        }
    }

    CloseHandle(hProcess);
    return found;
}

bool IsAlreadyInjected(DWORD pid, const std::wstring& dllName) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return false;

    HMODULE hMods[1024];
    DWORD cbNeeded;
    bool injected = false;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i) {
            wchar_t modName[MAX_PATH];
            if (GetModuleBaseNameW(hProcess, hMods[i], modName, MAX_PATH)) {
                if (_wcsicmp(modName, dllName.c_str()) == 0) {
                    injected = true;
                    break;
                }
            }
        }
    }

    CloseHandle(hProcess);
    return injected;
}

bool InjectDLL(DWORD pid, const std::wstring& dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;

    size_t size = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID allocMem = VirtualAllocEx(hProcess, nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!allocMem) { CloseHandle(hProcess); return false; }

    if (!WriteProcessMemory(hProcess, allocMem, dllPath.c_str(), size, nullptr)) {
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandleW(L"Kernel32");
    if (!hKernel32) {
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    FARPROC loadLib = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!loadLib) {
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)loadLib, allocMem, 0, nullptr);

    if (!hThread) {
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}
