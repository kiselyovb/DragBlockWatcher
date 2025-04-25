#pragma once

#include <string>

// ��������: �������� �� ������� GUI (�������� user32.dll)
bool IsGuiProcess(DWORD pid);

// ���������� �������� DLL � ��������� �������
bool InjectDLL(DWORD pid, const std::wstring& dllPath);

// (�����������) ��������, �������� �� DLL ���
bool IsAlreadyInjected(DWORD pid, const std::wstring& dllName);
