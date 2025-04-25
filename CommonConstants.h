#pragma once

// ��������� ��� ������ � �����������
const wchar_t* const kTargetDllName = L"DragBlock.dll";
const wchar_t* const kRegistryKeyPath = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
const wchar_t* const kRegistryValueName = L"AppInit_DLLs";
const wchar_t* const kWmiNamespace = L"ROOT\\CIMV2";
const wchar_t* const kWmiQuery = L"SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'";

// ��������� ��� ������ � �������
const wchar_t* const kServiceName = L"DragBlockWatcher";
const wchar_t* const kServiceDisplayName = L"���������� Drag & Drop";
const wchar_t* const kServiceDescription = L"������ ��� �������������� �������������� ������ ����� ������";
const wchar_t* const kServiceBinaryPath = L"C:\\Program Files\\DragBlock\\DragBlockWatcher.exe";

