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

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)
#define TEST_BIT(nr, array) \
   ((1UL << ((nr) % LONG_BITS)) & (array)[(nr) / LONG_BITS])

static struct {
	int ifd;
	int gfd;
	int grabbed;
	void (*on_button)(int button);
} gamepad;

/// Filter for scandir. We only care about /dev/input/event* devices.
static int is_event_device(const struct dirent *dir) {
	return strncmp("event", dir->d_name, 5) == 0;
}

static int gamepad_open()
{
	int fd = -1;

	struct dirent **namelist;
	int num_devices = scandir("/dev/input", &namelist, is_event_device, alphasort);
	if (num_devices < 0) {
		perror("scandir");
		return -1;
	}

	for (int i = 0; i < num_devices; i++) {
		LOG("%d/%d: %s", i, num_devices, namelist[i]->d_name);

		char path[300];
		snprintf(path, sizeof(path), "/dev/input/%s", namelist[i]->d_name);

		int current_fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (current_fd < 0) continue;

		unsigned long key_bits[NLONGS(KEY_CNT)] = {0};
		if (ioctl(current_fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) < 0) {
			perror("EVIOCGBIT");
		}

		if (TEST_BIT(BTN_GAMEPAD, key_bits)) {
			fd = current_fd;
			break;
		}

		close(current_fd);
	}

	for (int i = 0; i < num_devices; i++) {
		free(namelist[i]);
	}

	free(namelist);

	return fd;
}

void gamepad_init(int grab, void (*on_button)(int button))
{
	gamepad.ifd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	inotify_add_watch(gamepad.ifd, "/dev/input", IN_CREATE);

	gamepad.gfd = gamepad_open();
	if (grab) gamepad_grab();

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

void gamepad_dispatch()
{
	// Try to read inotify
	char buf[sizeof(struct inotify_event) + NAME_MAX + 1];
	int n = read(gamepad.ifd, buf, sizeof(buf));

	if (n > 0) {
		// A new /dev/input/event* device showed up.
		// Try to open it and restore grab.
		if (gamepad.gfd < 0) {
			gamepad.gfd = gamepad_open();
			if (gamepad.grabbed) gamepad_grab();
		}
	}
	else if (n < 0 && errno != EWOULDBLOCK) {
		LOG("Failed to read inotify");
		return;
	}

	// Try to read gamepad
	struct input_event ev;
	n = read(gamepad.gfd, &ev, sizeof(ev));

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
