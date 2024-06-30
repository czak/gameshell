#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/inotify.h>

#include "gamepad.h"
#include "log.h"

static struct {
	int ifd;
	int gfd;
	int grabbed;
	void (*on_gamepad)(void);
	void (*on_button)(int button);
} gamepad;

/**
 * Find "event%d" in the given string and return the number.
 * Return -1 if string does not contain "event";
 */
static int parse_handlers(char *line)
{
	char *event = strstr(line, "event");
	if (!event) return -1;

	int n = -1;
	sscanf(event, "event%d", &n);
	return n;
}

/**
 * Convert a single hex digit character to a number
 *
 * Input must be one of 0123456789abcdef.
 */
static int parse_digit(char c)
{
	return c >= 'a' ? c - 87 : c - 48;
}

/**
 * Copy a double-quoted string from src to dst, up to size.
 * Adds a terminating '\0', so actual string will be up to size - 1.
 */
static void parse_name(char *dst, char *src, int size)
{
	char *start = strchr(src, '"');
	if (!start) return;

	char *end = strrchr(src, '"');
	if (!end) return;

	size_t len = end - start - 1;
	if (len > size - 1) len = size - 1;

	strncpy(dst, start + 1, len);
	dst[len] = '\0';
}

static int is_gamepad(char *line)
{
	char *words[12] = {0};
	int num_words = 0;

	// Read in all the words, starting right after "B: KEY="
	for (char *word = strtok(line + 7, " \n"); word != NULL; word = strtok(NULL, " \n")) {
		words[num_words++] = word;
	}

	// Which word contains the BTN_GAMEPAD bit
	int w = num_words - BTN_GAMEPAD / 64 - 1;
	if (w < 0) return 0;

	// Which nibble in the word contains the BTN_GAMEPAD bit
	int n = strnlen(words[w], 16) - (BTN_GAMEPAD % 64) / 4 - 1;
	if (n < 0) return 0;

	return parse_digit(words[w][n]) & (1 << (BTN_GAMEPAD % 4));
}

static int gamepad_open()
{
	FILE *f = fopen("/proc/bus/input/devices", "r");
	char buf[256], name[64];
	int id = -1, fd = -1;

	while (fgets(buf, sizeof(buf), f)) {
		if (strstr(buf, "N: Name=")) {
			id = -1;
			parse_name(name, buf, sizeof(name));
		} else if (strstr(buf, "H: Handlers=")) {
			id = parse_handlers(buf);
		} else if (id != -1 && strstr(buf, "B: KEY=")) {
			if (is_gamepad(buf)) {
				snprintf(buf, sizeof(buf), "/dev/input/event%d", id);

				fd = open(buf, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
				if (fd >= 0) {
					LOG("Found %s at %s", name, buf);
					break;
				}
			}
		}
	}

	fclose(f);
	return fd;
}

void gamepad_init(void (*on_gamepad)(void), void (*on_button)(int button))
{
	gamepad.ifd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	inotify_add_watch(gamepad.ifd, "/dev/input", IN_CREATE | IN_ATTRIB);

	gamepad.gfd = gamepad_open();
	gamepad_grab();

	gamepad.on_gamepad = on_gamepad;
	gamepad.on_button = on_button;
}

int gamepad_get_inotify()
{
	return gamepad.ifd;
}

int gamepad_get_fd()
{
	return gamepad.gfd;
}

char *gamepad_get_name()
{
	if (gamepad.gfd > 0) {
		return "connected";
	} else {
		return "not connected";
	}
}

static void dispatch_ifd()
{
	char buf[sizeof(struct inotify_event) + NAME_MAX + 1];
	int n = read(gamepad.ifd, buf, sizeof(buf));

	if (n > 0) {
		// A new /dev/input/event* device showed up.
		// Try to open it and restore grab.
		if (gamepad.gfd < 0) {
			gamepad.gfd = gamepad_open();
			if (gamepad.grabbed) gamepad_grab();
			if (gamepad.on_gamepad) gamepad.on_gamepad();
		}
	}
	else if (n < 0 && errno != EWOULDBLOCK) {
		LOG("Failed to read inotify");
		return;
	}
}

static void dispatch_gfd()
{
	struct input_event ev;
	int n = read(gamepad.gfd, &ev, sizeof(ev));

	if (n > 0) {
		if (gamepad.on_button) {
			if (ev.type == EV_KEY && ev.value == 1)
				gamepad.on_button(ev.code);

			// Translate "analog" dpad to digital
			// See https://www.kernel.org/doc/html/latest/input/gamepad.html
			if (ev.type == EV_ABS && ev.value != 0) {
				if (ev.code == ABS_HAT0X) {
					if (ev.value == -1) gamepad.on_button(BTN_DPAD_LEFT);
					if (ev.value == 1) gamepad.on_button(BTN_DPAD_RIGHT);
				} else if (ev.code == ABS_HAT0Y) {
					if (ev.value == -1) gamepad.on_button(BTN_DPAD_UP);
					if (ev.value == 1) gamepad.on_button(BTN_DPAD_DOWN);
				}
			}
		}
	}
	else if (n < 0 && errno != EWOULDBLOCK) {
		LOG("Failed to read gamepad");
		close(gamepad.gfd);
		gamepad.gfd = gamepad_open();
		if (gamepad.grabbed) gamepad_grab();
		if (gamepad.on_gamepad) gamepad.on_gamepad();
	}
}

void gamepad_dispatch()
{
	if (gamepad.ifd >= 0) {
		dispatch_ifd();
	}

	if (gamepad.gfd >= 0) {
		dispatch_gfd();
	}
}

void gamepad_grab()
{
	if (gamepad.gfd < 0) return;

	int res = ioctl(gamepad.gfd, EVIOCGRAB, 1);
	if (res == 0) {
		gamepad.grabbed = GAMEPAD_GRABBED;
	}
	else if (res < 0) {
		perror("EVIOCGRAB");
	}
}

void gamepad_ungrab()
{
	if (gamepad.gfd < 0) return;

	int res = ioctl(gamepad.gfd, EVIOCGRAB, 0);
	if (res == 0) {
		gamepad.grabbed = GAMEPAD_UNGRABBED;
	}
	else if (res < 0) {
		perror("EVIOCGRAB");
	}
}
