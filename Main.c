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

DWM_THUMBNAIL_PROPERTIES gThumbnailProperties = { 0 };

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

	DWORD RefreshTime = 0;

	WindowClass.cbSize = sizeof(WNDCLASSEXW);

	WindowClass.lpfnWndProc = MainWindowProc;	

	WindowClass.lpszClassName = L"KeepAnEyeOutWindowClass";

	WindowClass.style = CS_HREDRAW | CS_VREDRAW;

	WindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;

	WindowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);

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
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
		case WM_CREATE:
		{
			HFONT Font = CreateFontW(14, 0, 0, 0, 200, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");

			wchar_t ErrorMessage[256] = { 0 };

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
				WindowHandle,
				(HMENU)LABEL01_ID,
				GetModuleHandleW(NULL),
				NULL);

			if (gLabel01 == NULL)
			{
				_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to create label! Error: 0x%08lx", GetLastError());

				MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

				gAppIsRunning = FALSE;

				break;
			}

			SendMessageW(gLabel01, WM_SETFONT, (WPARAM)Font, 0);

			// Create the dropdown box.

			gComboBox01 = CreateWindowExW(
				0,
				L"COMBOBOX",
				NULL,
				CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL,
				4,
				24,
				350,
				300,
				WindowHandle,
				(HMENU)COMBOBOX01_ID,
				GetModuleHandleW(NULL),
				NULL);

			if (gComboBox01 == NULL)
			{
				_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to create combobox! Error: 0x%08lx", GetLastError());

				MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

				gAppIsRunning = FALSE;

				break;
			}

			SendMessageW(gComboBox01, WM_SETFONT, (WPARAM)Font, 0);

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
				WindowHandle,
				(HMENU)BUTTON01_ID,
				GetModuleHandleW(NULL),
				NULL);

			if (gButton01 == NULL)
			{
				_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to create button! Error: 0x%08lx", GetLastError());

				MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

				gAppIsRunning = FALSE;

				break;
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
				_snwprintf_s(ErrorMessage, _countof(ErrorMessage), _TRUNCATE, L"Failed to load refresh bitmap! Error: 0x%08lx", GetLastError());

				MessageBoxW(NULL, ErrorMessage, L"ERROR", MB_ICONERROR | MB_OK);

				gAppIsRunning = FALSE;

				break;
			}

			SendMessageW(gButton01, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)gRefreshButtonImage);

			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(WParam))
			{
				case BUTTON01_ID:
				{
					if (gThumbnail != NULL)
					{
						DwmUnregisterThumbnail(gThumbnail);

						gThumbnail = NULL;
					}

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
							RECT ClientRect = { 0 };

							GetClientRect(gMainWindow, &ClientRect);

							ClientRect.left += 4;

							ClientRect.right -= 4;

							ClientRect.bottom -= 4;

							ClientRect.top += 50;

							gThumbnailProperties.dwFlags = DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_VISIBLE | DWM_TNP_OPACITY | DWM_TNP_RECTDESTINATION;

							gThumbnailProperties.fSourceClientAreaOnly = FALSE;

							gThumbnailProperties.fVisible = TRUE;

							gThumbnailProperties.opacity = 255;
							
							// IMPORTANT: Per-Monitor DPI Awareness set in application manifest!
							gThumbnailProperties.rcDestination = ClientRect;

							Hr = DwmUpdateThumbnailProperties(gThumbnail, &gThumbnailProperties);

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
		case WM_LBUTTONUP:
		{
			if (gThumbnail != NULL)
			{
				wchar_t WindowText[256] = { 0 };

				DWORD SelectedIndex = (DWORD)SendMessageW(gComboBox01, CB_GETCURSEL, 0, 0);

				SendMessageW(gComboBox01, CB_GETLBTEXT, (WPARAM)SelectedIndex, (LPARAM)WindowText);

				BringWindowToTop(FindWindowW(NULL, WindowText));
			}

			break;
		}
		case WM_SIZE:
		case WM_SIZING:
		{
			if (gThumbnail != NULL)
			{
				RECT ClientRect = { 0 };

				GetClientRect(gMainWindow, &ClientRect);

				ClientRect.left += 4;

				ClientRect.right -= 4;

				ClientRect.bottom -= 4;

				ClientRect.top += 50;

				gThumbnailProperties.rcDestination = ClientRect;

				DwmUpdateThumbnailProperties(gThumbnail, &gThumbnailProperties);
			}

			return(TRUE);
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

		if ((wcslen(WindowTitle) > 0) && (WindowHandle != gMainWindow) && IsWindowVisible(WindowHandle))
		{
			SendMessageW(gComboBox01, CB_ADDSTRING, 0, (LPARAM)WindowTitle);			
		}
	}

	return(TRUE);
}