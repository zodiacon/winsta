// winsta.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#pragma comment(lib, "wtsapi32")

enum class WindowStationState {
	Active = 0,
	Connected = 1,
	ConnectQuery = 2,
	Shadow = 3,
	Disconnected = 4,
	Idle = 5,
	Listen = 6,
	Reset = 7,
	Down = 8,
	Init = 9
};

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

const char* StateToString(WindowStationState state) {
	switch (state) {
		case WindowStationState::Active: return "Active";
		case WindowStationState::Connected: return "Connected";
		case WindowStationState::ConnectQuery: return "ConnectQuery";
		case WindowStationState::Shadow: return "Shadow";
		case WindowStationState::Disconnected: return "Disconnected";
		case WindowStationState::Idle: return "Idle";
		case WindowStationState::Listen: return "Listen";
		case WindowStationState::Reset: return "Reset";
		case WindowStationState::Down: return "Down";
		case WindowStationState::Init: return "Init";
	}
	return "Unknown";
}

void EnumSessions() {
	DWORD level = 1;
	PWTS_SESSION_INFO_1 info;
	DWORD count = 0;
	if (!::WTSEnumerateSessionsEx(nullptr, &level, 0, &info, &count)) {
		printf("Error enumerating sessions (%d)\n", ::GetLastError());
		return;
	}

	for (DWORD i = 0; i < count; i++) {
		auto& data = info[i];
		printf("Session %d (%ws) Username: %ws\\%ws State: %s\n", data.SessionId, data.pSessionName, 
			data.pDomainName ? data.pDomainName : L"NT AUTHORITY", data.pUserName ? data.pUserName : L"SYSTEM", StateToString((WindowStationState)data.State));
	}

	::WTSFreeMemory(info);
}

void EnumWinStations() {
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

}

int main() {
	EnumSessions();
	printf("\n");
	EnumWinStations();

	return 0;
}

