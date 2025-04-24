#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>

#include "log.h"

/**
 * Ensure only a single instance of the program is running
 */
void system_set_lockfile()
{
	const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
	if (!runtime_dir) {
		runtime_dir = "/tmp";
	}

	char lockfile[128];
	int len = snprintf(lockfile, sizeof(lockfile), "%s/%s", runtime_dir, "gameshell.lock");
	assert(len >= 0 && len < sizeof(lockfile));

	int fd = open(lockfile, O_CREAT | O_RDWR, 0666);
	if (fd == -1) {
		log_fatal("Failed to create lockfile %s", lockfile);
	}

	if (flock(fd, LOCK_EX | LOCK_NB)) {
		log_fatal("Failed to lock %s. Is gameshell running?", lockfile);
	}
}
