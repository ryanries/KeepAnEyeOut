// KeepAnEyeOut
// Joseph Ryan Ries - 2022
// https://github.com/ryanries/KeepAnEyeOut
// This app lets you keep an eye on another app in the background while you are multitasking.
// I originally created this app so I could keep an eye on my MMORPG while I was camping a mob.
// This way I could see when the mob had respawned even if I was browsing the web or working on something else.
// Important: Per-monitor DPI awareness should be set in the application manifest.
// 

#include <windows.h>

#include <stdio.h>

#include <dwmapi.h>

#include "Main.h"

#include "resource.h"

#pragma comment(lib, "dwmapi.lib")

HWND gMainWindow;

HWND gLabel01;

HWND gComboBox01;

HWND gButton01;

HBITMAP gRefreshButtonImage;

HTHUMBNAIL gThumbnail;

BOOL gAppIsRunning = TRUE;



int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);

	UNREFERENCED_PARAMETER(hPrevInstance);

	UNREFERENCED_PARAMETER(pCmdLine);

	UNREFERENCED_PARAMETER(nCmdShow);

	DWORD Result = ERROR_SUCCESS;

	wchar_t ErrorMessage[256] = { 0 };

	MSG WindowMessage = { 0 };

	WNDCLASSEXW WindowClass = { 0 };

	HFONT GUIFont = CreateFontW(14, 0, 0, 0, 200, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");

	DWORD RefreshTime = 0;

	WindowClass.cbSize = sizeof(WNDCLASSEXW);

	WindowClass.lpfnWndProc = MainWindowProc;	

	WindowClass.lpszClassName = L"KeepAnEyeOutWindowClass";

	WindowClass.style = CS_HREDRAW | CS_VREDRAW;

	WindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;

	// Create the main window.

	if (RegisterClassExW(&WindowClass) == 0)
	{
		Result = GetLastError();

		_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to register window class! Error: 0x%08lx", Result);

		MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

		goto Exit;
	}

	gMainWindow = CreateWindowExW(
		WS_EX_TOPMOST,
		WindowClass.lpszClassName,
		L"KeepAnEyeOut",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		412,
		320,
		NULL,
		NULL,
		GetModuleHandleW(NULL),
		NULL);

	if (gMainWindow == NULL)
	{
		Result = GetLastError();

		_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to create window! Error: 0x%08lx", Result);

		MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

		goto Exit;
	}

	// Create the static label.

	gLabel01 = CreateWindowExW(
		0,
		L"STATIC",
		L"Choose a window to keep an eye on:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		4,
		4,
		300,
		20,
		gMainWindow,
		(HMENU)LABEL01_ID,
		GetModuleHandleW(NULL),
		NULL);

	if (gLabel01 == NULL)
	{
		Result = GetLastError();

		_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to create label! Error: 0x%08lx", Result);

		MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

		goto Exit;
	}

	SendMessageW(gLabel01, WM_SETFONT, (WPARAM)GUIFont, 0);

	// Create the dropdown box.

	gComboBox01 = CreateWindowExW(
		0,
		L"COMBOBOX",
		NULL,
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		4, 
		24, 
		350, 
		300, 
		gMainWindow, 
		(HMENU)COMBOBOX01_ID, 
		GetModuleHandleW(NULL),
		NULL);

	if (gComboBox01 == NULL)
	{
		Result = GetLastError();

		_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to create combobox! Error: 0x%08lx", Result);

		MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

		goto Exit;
	}

	SendMessageW(gComboBox01, WM_SETFONT, (WPARAM)GUIFont, 0);

	EnumWindows(EnumWindowsProc, 0);

	// Create the refresh button.

	gButton01 = CreateWindowExW(
		0,
		L"BUTTON",
		NULL,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_BITMAP,
		360,
		22,
		24,
		24,
		gMainWindow,
		(HMENU)BUTTON01_ID,
		GetModuleHandleW(NULL),
		NULL);

	if (gButton01 == NULL)
	{
		Result = GetLastError();

		_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to create button! Error: 0x%08lx", Result);

		MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

		goto Exit;
	}

	gRefreshButtonImage = (HBITMAP)LoadImageW(
		GetModuleHandleW(NULL),
		MAKEINTRESOURCEW(IDB_BITMAP1),
		IMAGE_BITMAP,
		0,
		0,
		LR_DEFAULTCOLOR);

	if (gRefreshButtonImage == NULL)
	{
		Result = GetLastError();

		_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to load refresh bitmap! Error: 0x%08lx", Result);

		MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

		goto Exit;
	}

	SendMessageW(gButton01, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)gRefreshButtonImage);

	while (gAppIsRunning == TRUE)
	{
		// PeekMessageW(&WindowMessage, gMainWindow, 0, 0, PM_REMOVE) handles messages for the main window ONLY!
		// By using NULL, we can process messages for the main window and all of the child windows/controls.

		while (PeekMessageW(&WindowMessage, NULL, 0, 0, PM_REMOVE))
		{
			DispatchMessageW(&WindowMessage);
		}

		// Periodically re-set the window to top-most, to prevent the window from being hidden
		// underneath other top-most windows, for example, a fullscreen RDP session or fullscreen video.
		// However, do NOT do this if the combobox is currently dropped down, because this action causes
		// the combobox to collapse if dropped down, and that would really annoy the user if the user 
		// was trying to interact with the combobox when this happened.
		if ((RefreshTime > 2000) && (SendMessageW(gComboBox01, CB_GETDROPPEDSTATE, 0, 0) == FALSE))
		{			
			RefreshTime = 0;

			SetWindowPos(gMainWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}

		RefreshTime += 10;

		Sleep(10);
	}

Exit:

	return(Result);
}

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
		case WM_COMMAND:
		{
			switch (LOWORD(WParam))
			{
				case BUTTON01_ID:
				{
					SendMessageW(gComboBox01, CB_RESETCONTENT, 0, 0);

					EnumWindows(EnumWindowsProc, 0);

					break;
				}
				case COMBOBOX01_ID:
				{
					if (HIWORD(WParam) == CBN_SELCHANGE)
					{
						HRESULT Hr = S_OK;						

						wchar_t ErrorMessage[256] = { 0 };

						wchar_t WindowText[256] = { 0 };

						DWORD SelectedIndex = 0;

						// First we unregister any previous DWM thumbnail, or else we leak.

						if (gThumbnail != NULL)
						{
							Hr = DwmUnregisterThumbnail(gThumbnail);

							gThumbnail = NULL;

							if (Hr != S_OK)
							{
								_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"DwmUnRegisterThumbnail failed! HRESULT = %08lx", Hr);

								MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_OK | MB_ICONERROR);

								break;
							}
						}

						//

						SelectedIndex = (DWORD)SendMessageW(gComboBox01, CB_GETCURSEL, 0, 0);

						SendMessageW(gComboBox01, CB_GETLBTEXT, (WPARAM)SelectedIndex, (LPARAM)WindowText);

						Hr = DwmRegisterThumbnail(
							gMainWindow, 
							FindWindowW(NULL, WindowText),
							&gThumbnail);

						if (SUCCEEDED(Hr))
						{
							DWM_THUMBNAIL_PROPERTIES ThumbnailProperties = { 0 };

							ThumbnailProperties.dwFlags = DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_VISIBLE | DWM_TNP_OPACITY | DWM_TNP_RECTDESTINATION;

							ThumbnailProperties.fSourceClientAreaOnly = FALSE;

							ThumbnailProperties.fVisible = TRUE;

							ThumbnailProperties.opacity = 255;

							// 384x216 resolution - 16:9 aspect ratio
							// IMPORTANT: Per-Monitor DPI Awareness set in application manifest!
							ThumbnailProperties.rcDestination = (RECT){ 4, 48, 384 + 4, 216 + 48 }; 

							Hr = DwmUpdateThumbnailProperties(gThumbnail, &ThumbnailProperties);

							if (FAILED(Hr))
							{
								_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"DwmUpdateThumbnailProperties failed! HRESULT = %08lx", Hr);

								MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_OK | MB_ICONERROR);
							}
						}
						else
						{
							_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"DwmRegisterThumbnail failed! HRESULT = %08lx", Hr);
															
							MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_OK | MB_ICONERROR);
						}
					}

					break;
				}
				default:
				{

				}
			}

			break;
		}
		case WM_CLOSE:
		{
			gAppIsRunning = FALSE;

			PostQuitMessage(0);

			break;
		}
		default:
		{
			Result = DefWindowProcW(WindowHandle, Message, WParam, LParam);
		}
	}

	return(Result);
}

BOOL CALLBACK EnumWindowsProc(_In_ HWND WindowHandle, _In_ LPARAM LParam)
{	
	UNREFERENCED_PARAMETER(LParam);

	if (IsWindowVisible(WindowHandle))
	{
		wchar_t WindowTitle[256] = { 0 };

		GetWindowTextW(WindowHandle, WindowTitle, _countof(WindowTitle));

		if ((wcslen(WindowTitle) > 0) && (WindowHandle != gMainWindow))
		{
			SendMessageW(GetDlgItem(gMainWindow, COMBOBOX01_ID), CB_ADDSTRING, 0, (LPARAM)WindowTitle);			
		}
	}

	return(TRUE);
}