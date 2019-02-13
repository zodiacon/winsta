// winsta.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

void DoEnumDesktopWindows(PCWSTR name) {
	auto hdesk = ::OpenDesktop(name, 0, FALSE, DESKTOP_ENUMERATE | DESKTOP_READOBJECTS);
	if (!hdesk) {
		printf("--- failed to open desktop %ws (%d)\n", name, ::GetLastError());
		return;
	}
	::EnumDesktopWindows(hdesk, [](auto hwnd, auto) -> BOOL {
		static WCHAR text[128];
		if (::IsWindowVisible(hwnd) && ::GetWindowText(hwnd, text, 128) > 0) {
			printf("  HWND: 0x%08X: %ws\n", (DWORD)(DWORD_PTR)hwnd, text);
		}
		return TRUE;
		}, 0);
	::CloseDesktop(hdesk);
}

int main() {
	::EnumWindowStations([](auto name, auto) -> BOOL {
		printf("Window station: %ws\n", name);
		auto hWinSta = ::OpenWindowStation(name, FALSE, WINSTA_ENUMDESKTOPS);
		if (!hWinSta) {
			printf("Error opening Winsta (%d)\n", ::GetLastError());
		}
		else {
			::EnumDesktops(hWinSta, [](auto deskname, auto) -> BOOL {
				printf(" Desktop: %ws\n", deskname);
				DoEnumDesktopWindows(deskname);
				return TRUE;
			}, 0);
			::CloseWindowStation(hWinSta);
		}
		return TRUE;
	}, 0);

	return 0;
}

