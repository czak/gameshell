#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void log_print(const char *file, int lineno, const char *fmt, ...)
{
	struct timespec ts;
	clock_gettime(CLOCK_BOOTTIME, &ts);

	static time_t start_sec = -1;
	if (start_sec == -1) {
		start_sec = ts.tv_sec;
	}

	fprintf(stderr, "\033[2m");
	fprintf(stderr, "[%3ld.%03ld] %s:%d: ", (ts.tv_sec - start_sec) % 1000,
			ts.tv_nsec / 1000000, file, lineno);
	fprintf(stderr, "\033[0m");

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, "\n");
}
