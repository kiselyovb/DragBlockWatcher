#include "pch.h"

// ServiceMain.cpp
#include <windows.h>
#include <tchar.h>
#include "WMIWatcher.h"

SERVICE_STATUS g_ServiceStatus = {};
SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
HANDLE g_StopEvent = nullptr;

void WINAPI ServiceMain(DWORD, LPTSTR*);
void WINAPI ServiceCtrlHandler(DWORD);

int wmain() {
    SERVICE_TABLE_ENTRY ServiceTable[] = {
        { (LPWSTR)L"DragBlockWatcher", (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { nullptr, nullptr }
    };

    if (!StartServiceCtrlDispatcher(ServiceTable)) {
        return 1;
    }
    return 0;
}

void WINAPI ServiceMain(DWORD, LPTSTR*) {
    g_StatusHandle = RegisterServiceCtrlHandler(L"DragBlockWatcher", ServiceCtrlHandler);
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;

    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
    g_StopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    StartWMIWatcher();  // запуск WMI-модуля

    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    WaitForSingleObject(g_StopEvent, INFINITE);

    StopWMIWatcher();  // остановка
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
    if (CtrlCode == SERVICE_CONTROL_STOP) {
        SetEvent(g_StopEvent);
    }
}
