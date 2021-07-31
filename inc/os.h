#ifndef OS_H
#define OS_H

#define LINUX 0x0005
#define WIN32 0x1001
#define WIN64 0x1011

#if defined(__linux__)
	#define OS LINUX
#elif defined(_WIN64)
	#define OS WIN64
#elif defined(_WIN32)
	#define OS WIN32
#endif

#endif // OS_H
