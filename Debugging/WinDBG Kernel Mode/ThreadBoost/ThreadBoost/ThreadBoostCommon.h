#pragma once

struct ThreadData {
	ULONG ThreadId;
	int Priority;
};

#define IOCTL_THREAD_BOOST_PRIORITY CTL_CODE(0x8000, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
