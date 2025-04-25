
# DragBlockWatcher � �������� ������ ������

`DragBlockWatcher` � ��� ������ Windows, ������� ������������� ����������� ��������� ����� ��������� � �������� � ��� �������� ���������� `DragBlock.dll`, ���� ������� �������� GUI-�����������. ��� ���������� ��������� �������������� (Drag & Drop) ������ ����� ������.

---

## ����� �����������

��������� ������� �� ��������� �������:

- `ServiceMain.cpp`: ����� ����� � ���������� ��������� ������ ������.
- `WMIWatcher.cpp`: �������� �� ������� �������� ����� ��������� ����� WMI.
- `InjectorLogic.cpp`: ������ ��������� DLL � GUI-��������.

---

## �������� ������� ������

1. **����������� ������ � �������**
   - ������ ����������� ������������� ��� �������.
   - ���� � `ServiceMain`, ���:
     - �������������� ���������� ���������� (Stop).
     - ��������������� ������ ������ � `SERVICE_RUNNING`.

2. **������������� WMI-��������**
   - COM ����������������.
   - ��������������� ���������� � ������������� `ROOT\CIMV2`.
   - �������������� �������� �� `__InstanceCreationEvent`:
     ```sql
     SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'
     ```

3. **��������� ������� �������� ��������**
   - ��� ������� �������:
     - ����������� `ProcessId`.
     - �����������, �������� �� ������� GUI-����������� (`user32.dll`).
     - �����������, �������� �� ��� `DragBlock.dll`.
     - ���� ��� � ����������� `InjectDLL(...)` ����� `CreateRemoteThread`.

---

## ������ ����������� GUI-���������

GUI-���������� ������������ �� ������� ������������ ������ `user32.dll`:
```cpp
EnumProcessModules(...)
GetModuleBaseNameW(...) == "user32.dll"
```

---

## �������� DLL

����� ��������:
1. ��������� ������ � ������� �������� ����� `VirtualAllocEx`.
2. ������ ���� � DLL ����� `WriteProcessMemory`.
3. ��������� ������ `LoadLibraryW`.
4. �������� ������ ����� `CreateRemoteThread`.

---

## ���������� ������

- ������ ����� ���� ����������� ����� `sc stop DragBlockWatcher`.
- ��� ���� ���������� `StopWMIWatcher()`, �������� ����� � ��������� WMI.
- COM �������������, � ������ ������ �������� �� `SERVICE_STOPPED`.

---

## �������� � ��������������

- �������� ���������� ����� Process Explorer / Process Hacker.
- � ����� ����������� �������� �������� (���� �������� �����������).
- ��� ������ `DragBlock.dll` ������ ���� � ������ ����������� DLL ��������.

---

## �����������

- ��������� ���������������� �����.
- DLL ������ ���� ��������� � ��������� ��������.
- ������ �������� ������ � ��������, ��� �������� WMI (`ROOT\CIMV2`).

---

## �������� �������

```cmd
sc create DragBlockWatcher binPath= "C:\Program Files\DragBlock\DragBlockWatcher.exe" start= auto DisplayName= "���������� Drag & Drop" 
sc description DragBlockWatcher "������ ��� �������������� �������������� ������ ����� ������"
sc start DragBlockWatcher
sc stop DragBlockWatcher
sc delete DragBlockWatcher
```

---
