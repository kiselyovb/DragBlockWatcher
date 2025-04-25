
# DragBlockWatcher — Алгоритм работы службы

`DragBlockWatcher` — это служба Windows, которая автоматически отслеживает появление новых процессов и внедряет в них защитную библиотеку `DragBlock.dll`, если процесс является GUI-приложением. Эта библиотека блокирует перетаскивание (Drag & Drop) текста между окнами.

---

## Общая архитектура

Программа состоит из следующих модулей:

- `ServiceMain.cpp`: точка входа и управление жизненным циклом службы.
- `WMIWatcher.cpp`: подписка на события создания новых процессов через WMI.
- `InjectorLogic.cpp`: логика внедрения DLL в GUI-процессы.

---

## Алгоритм запуска службы

1. **Регистрация службы в системе**
   - Служба запускается автоматически или вручную.
   - Вход в `ServiceMain`, где:
     - Регистрируется обработчик управления (Stop).
     - Устанавливается статус службы в `SERVICE_RUNNING`.

2. **Инициализация WMI-подписки**
   - COM инициализируется.
   - Устанавливается соединение с пространством `ROOT\CIMV2`.
   - Регистрируется подписка на `__InstanceCreationEvent`:
     ```sql
     SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'
     ```

3. **Обработка события создания процесса**
   - Для каждого события:
     - Извлекается `ProcessId`.
     - Проверяется, является ли процесс GUI-приложением (`user32.dll`).
     - Проверяется, внедрена ли уже `DragBlock.dll`.
     - Если нет — выполняется `InjectDLL(...)` через `CreateRemoteThread`.

---

## Логика определения GUI-процессов

GUI-приложения определяются по наличию загруженного модуля `user32.dll`:
```cpp
EnumProcessModules(...)
GetModuleBaseNameW(...) == "user32.dll"
```

---

## Инъекция DLL

Метод инъекции:
1. Выделение памяти в целевом процессе через `VirtualAllocEx`.
2. Запись пути к DLL через `WriteProcessMemory`.
3. Получение адреса `LoadLibraryW`.
4. Создание потока через `CreateRemoteThread`.

---

## Завершение работы

- Служба может быть остановлена через `sc stop DragBlockWatcher`.
- При этом вызывается `StopWMIWatcher()`, завершая поток с подпиской WMI.
- COM освобождается, и статус службы меняется на `SERVICE_STOPPED`.

---

## Проверка и журналирование

- Проверка происходит через Process Explorer / Process Hacker.
- В логах фиксируются успешные инъекции (если включено логирование).
- Имя модуля `DragBlock.dll` должно быть в списке загруженных DLL процесса.

---

## Ограничения

- Требуются административные права.
- DLL должна быть устойчива к повторной загрузке.
- Служба работает только в системах, где включена WMI (`ROOT\CIMV2`).

---

## Полезные команды

```cmd
sc create DragBlockWatcher binPath= "C:\Program Files\DragBlock\DragBlockWatcher.exe" start= auto DisplayName= "Мониторинг Drag & Drop" 
sc description DragBlockWatcher "Служба для предотвращения перетаскивания текста между окнами"
sc start DragBlockWatcher
sc stop DragBlockWatcher
sc delete DragBlockWatcher
```

---
