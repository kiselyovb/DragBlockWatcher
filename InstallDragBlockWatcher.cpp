#include "pch.h"
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

int wmain() {
    SC_HANDLE hSCM = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) return 1;

    SC_HANDLE hService = CreateServiceW(
        hSCM,
        L"DragBlockWatcher",
        L"���������� Drag & Drop",           // ������������ ���
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        L"C:\\Program Files\\DragBlock\\DragBlockWatcher.exe", // ���� � EXE
        nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (hService) {
        SERVICE_DESCRIPTION desc;
        desc.lpDescription = (LPWSTR)L"������ ��� �������������� �������������� ������ ����� ������";
        ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &desc);
        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCM);
    return 0;
}
