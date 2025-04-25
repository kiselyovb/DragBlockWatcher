/*
Модуль WMIWatcher содержит логику, отвечающую за:
- Запуск и остановку WMI-подписки на создание новых процессов.
- Отслеживание событий __InstanceCreationEvent через WMI.
- Определение, является ли новый процесс GUI-приложением.
- Проверку, внедрена ли уже DLL, и выполнение инъекции при необходимости.
*/
#include "pch.h"

// WMIWatcher.cpp
#include "WMIWatcher.h"
#include "InjectorLogic.h"
#include <windows.h>
#include <wbemidl.h>
#include <comdef.h>
#include <thread>
#include <atomic>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "wbemuuid.lib")

static std::thread watcherThread;
static std::atomic<bool> running = false;

// Константы
const wchar_t* const kTargetDllName = L"DragBlock.dll";
const wchar_t* const kRegistryKeyPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
const wchar_t* const kRegistryValueName = L"AppInit_DLLs";
const wchar_t* const kWmiNamespace = L"ROOT\\CIMV2";
const wchar_t* const kWmiQuery = L"SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'";

std::wstring ResolveDLLPath() {
    HKEY hKey;
    wchar_t pathBuf[MAX_PATH] = { 0 };
    DWORD pathLen = sizeof(pathBuf);

    // Путь к ключу
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, kRegistryKeyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExW(hKey, L"AppInit_DLLs", NULL, NULL, (LPBYTE)pathBuf, &pathLen) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            if (PathFileExistsW(pathBuf)) {
                return std::wstring(pathBuf);
            }
        }
        RegCloseKey(hKey);
    }

    // Путь по умолчанию — рядом с EXE
    wchar_t modulePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);
    PathRemoveFileSpecW(modulePath);
    std::wstring fallbackPath = std::wstring(modulePath) + L"\\DragBlock.dll";

    if (PathFileExistsW(fallbackPath.c_str())) {
        return fallbackPath;
    }

    // Не найдено — вернём пустую строку
    return L"";
}

void StartWMIWatcher() {

    running = true;
    watcherThread = std::thread([] {
        CoInitializeEx(0, COINIT_MULTITHREADED);
        IWbemLocator* pLoc = nullptr;
        IWbemServices* pSvc = nullptr;

        CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID*)&pLoc);
        pLoc->ConnectServer(
            _bstr_t(kWmiNamespace), nullptr, nullptr, 0, NULL, 0, 0, &pSvc);
        CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
            nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr, EOAC_NONE);

        class EventSink : public IWbemObjectSink {
        public:
            ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
            ULONG STDMETHODCALLTYPE Release() override { return 1; }
            HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
                if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
                    *ppv = this;
                    return S_OK;
                }
                return E_NOINTERFACE;
            }

            HRESULT STDMETHODCALLTYPE Indicate(LONG lObjectCount, IWbemClassObject** apObjArray) override {
                for (int i = 0; i < lObjectCount; ++i) {
                    VARIANT vtPid;
                    if (SUCCEEDED(apObjArray[i]->Get(L"ProcessId", 0, &vtPid, nullptr, nullptr))) {
                        DWORD pid = vtPid.uintVal;

                        std::wstring dllPath = ResolveDLLPath();
                        if (dllPath.empty()) {
                            OutputDebugStringW(L"[DragBlockWatcher] DLL path not found. Injection skipped.");
                            VariantClear(&vtPid);
                            continue;
                        }

                        if (IsGuiProcess(pid) && !IsAlreadyInjected(pid, kTargetDllName)) {
                            InjectDLL(pid, dllPath);
                        }

                        VariantClear(&vtPid);
                    }
                }
                return WBEM_S_NO_ERROR;
            }

            HRESULT STDMETHODCALLTYPE SetStatus(LONG, HRESULT, BSTR, IWbemClassObject*) override {
                return WBEM_S_NO_ERROR;
            }
        };

        EventSink* pSink = new EventSink;
        pSvc->ExecNotificationQueryAsync(
            _bstr_t("WQL"),
            _bstr_t(kWmiQuery),
            WBEM_FLAG_SEND_STATUS, nullptr, pSink);

        while (running) Sleep(1000);

        pSvc->CancelAsyncCall(pSink);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        });
}

void StopWMIWatcher() {
    running = false;
    if (watcherThread.joinable())
        watcherThread.join();
}
