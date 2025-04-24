#pragma once

enum log_level {
	LOG_DEBUG = -1,
	LOG_INFO = 0,
	LOG_WARN = 1,
	LOG_ERROR = 2,
	LOG_FATAL = 10,
};

void log_print(enum log_level level, const char *fmt, ...);

#define log_debug(...) log_print(LOG_DEBUG, __VA_ARGS__)
#define log_info(...) log_print(LOG_INFO, __VA_ARGS__)
#define log_warn(...) log_print(LOG_WARN, __VA_ARGS__)
#define log_error(...) log_print(LOG_ERROR, __VA_ARGS__)
#define log_fatal(...) log_print(LOG_FATAL, __VA_ARGS__)
