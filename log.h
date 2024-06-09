#pragma once

void log_print(const char *file, int lineno, const char *fmt, ...);

#if defined(_LOG_ENABLE) && _LOG_ENABLE
	#define LOG(...) log_print(__FILE__, __LINE__, __VA_ARGS__);
#else
	#define LOG(...)
#endif
