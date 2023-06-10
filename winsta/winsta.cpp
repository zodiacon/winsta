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

void DoEnumDesktopWindows(HWINSTA hWinSta, PCWSTR name) {
	if (::SetProcessWindowStation(hWinSta)) {
		auto hdesk = ::OpenDesktop(name, 0, FALSE, DESKTOP_READOBJECTS);
		if (!hdesk) {
			printf("--- failed to open desktop %ws (%d)\n", name, ::GetLastError());
			return;
		}
		static WCHAR pname[MAX_PATH];
		::EnumDesktopWindows(hdesk, [](auto hwnd, auto) -> BOOL {
			static WCHAR text[128];
			if (::IsWindowVisible(hwnd) && ::GetWindowText(hwnd, text, 128) > 0) {
				DWORD pid;
				auto tid = ::GetWindowThreadProcessId(hwnd, &pid);
				auto hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
				BOOL exeNameFound = FALSE;
				PWSTR exeName = nullptr;
				if (hProcess) {
					DWORD size = MAX_PATH;
					exeNameFound = ::QueryFullProcessImageName(hProcess, 0, pname, &size);
					::CloseHandle(hProcess);
					if (exeNameFound) {
						exeName = ::wcsrchr(pname, L'\\');
						if (exeName == nullptr)
							exeName = pname;
						else
							exeName++;
					}
				}
				printf("  HWND: 0x%08X PID: 0x%X (%d) %ws TID: 0x%X (%d): %ws\n", (DWORD)(DWORD_PTR)hwnd, pid, pid, exeNameFound ? exeName : L"", tid, tid, text);
			}
			return TRUE;
			}, 0);
		::CloseDesktop(hdesk);
	}
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
	if (!::WTSEnumerateSessionsEx(WTS_CURRENT_SERVER_HANDLE, &level, 0, &info, &count)) {
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
		static HWINSTA hWinSta;
		hWinSta = ::OpenWindowStation(name, FALSE, WINSTA_ENUMDESKTOPS);
		if (!hWinSta) {
			printf("Error opening Window Station (%d)\n", ::GetLastError());
		}
		else {
			::EnumDesktops(hWinSta, [](auto deskname, auto) -> BOOL {
				printf(" Desktop: %ws\n", deskname);
				DoEnumDesktopWindows(hWinSta, deskname);
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

