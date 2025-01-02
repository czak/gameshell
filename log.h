#pragma once

void log_print(const char *file, int lineno, const char *fmt, ...);

#ifdef NDEBUG
	#define LOG(...)
#else
	#define LOG(...) log_print(__FILE__, __LINE__, __VA_ARGS__);
#endif
