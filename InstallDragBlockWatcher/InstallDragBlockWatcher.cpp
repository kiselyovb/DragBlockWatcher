#include "pch.h"
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "../CommonConstants.h"

int wmain() {
    SC_HANDLE hSCM = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) return 1;

    SC_HANDLE hService = CreateServiceW(
        hSCM,
        kServiceName,
        kServiceDisplayName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        kServiceBinaryPath,
        nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (hService) {
        SERVICE_DESCRIPTION desc;
        desc.lpDescription = (LPWSTR)kServiceDescription;
        ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &desc);
    }

    if (hService) {
        SERVICE_DESCRIPTION desc;
        desc.lpDescription = (LPWSTR)L"Служба для предотвращения перетаскивания текста между окнами";
        ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &desc);
        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCM);
    return 0;
}
