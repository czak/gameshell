#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

#ifdef NDEBUG
	static enum log_level min_level = LOG_INFO;
#else
	static enum log_level min_level = LOG_DEBUG;
#endif

void log_print(enum log_level level, const char *fmt, ...)
{
	if (level < min_level) return;

	const char *label = "";
	switch (level) {
		case LOG_DEBUG:
			label = "\033[90m[DBG]\033[0m ";
			break;
		case LOG_INFO:
			label = "\033[34m[INF]\033[0m ";
			break;
		case LOG_WARN:
			label = "\033[33m[WAR]\033[0m ";
			break;
		case LOG_ERROR:
		case LOG_FATAL:
			label = "\033[91m[ERR]\033[0m ";
			break;
	}

	fprintf(stderr, "%s", label);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");

	if (level == LOG_FATAL) exit(EXIT_FAILURE);
}
