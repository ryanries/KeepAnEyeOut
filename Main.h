#pragma once

#define LABEL01_ID		1001

#define COMBOBOX01_ID	1002

#define BUTTON01_ID		1003

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam);

BOOL CALLBACK EnumWindowsProc(_In_ HWND WindowHandle, _In_ LPARAM LParam);