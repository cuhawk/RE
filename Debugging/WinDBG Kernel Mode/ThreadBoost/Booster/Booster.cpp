// Booster.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "..\ThreadBoost\ThreadBoostCommon.h"


int Error(const char* msg) {
	printf("%s (%d)\n", msg, ::GetLastError());
	return 1;
}

int main(int argc, const char* argv[]) {
	if (argc < 3) {
		printf("Usage: booster <tid> <priority>\n");
		return 0;
	}

	HANDLE hFile = ::CreateFile(L"\\\\.\\ThreadBoost", GENERIC_READ | GENERIC_WRITE,
		0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return Error("Failed opening device");

	ThreadData data;
	data.ThreadId = atoi(argv[1]);
	data.Priority = atoi(argv[2]);

	DWORD bytes;
	if (!::DeviceIoControl(hFile, IOCTL_THREAD_BOOST_PRIORITY, &data, sizeof(data), nullptr, 0, &bytes, nullptr))
		return Error("failed in DeviceIoControl");


	::CloseHandle(hFile);
}

