#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

#define LONG_BITS (sizeof(long) * 8)
#define NLONGS(x) (((x) + LONG_BITS - 1) / LONG_BITS)
#define TEST_BIT(nr, array) \
   ((1UL << ((nr) % LONG_BITS)) & (array)[(nr) / LONG_BITS])

static int running = 1;

static int queue_init()
{
	return eventfd(0, 0);
}

static void queue_submit(int qfd, void (*cb)(int))
{
	pthread_t t;
	pthread_create(&t, NULL, (void *(*)(void *))cb, (void *)qfd);
	pthread_detach(t);
}

static int gamepad_open(const char *path)
{
	int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0) {
		return -1;
	};

	unsigned long key_bits[NLONGS(KEY_CNT)] = {0};
	if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) < 0) {
		perror("EVIOCGBIT");
	}

	if (TEST_BIT(BTN_GAMEPAD, key_bits)) {
		return fd;
	}

	// Not a gamepad
	close(fd);
	return -1;
}

static int is_event_device(const struct dirent *dir) {
	return strncmp("event", dir->d_name, 5) == 0;
}

static void gamepad_find(int qfd)
{
	struct dirent **namelist;
	int num_devices = scandir("/dev/input", &namelist, is_event_device, alphasort);
	if (num_devices < 0) {
		perror("scandir");
		return;
	}

	for (int i = 0; i < num_devices; i++) {
		char path[300];
		snprintf(path, sizeof(path), "/dev/input/%s", namelist[i]->d_name);

		fprintf(stderr, "trying %s\n", path);

		int fd = gamepad_open(path);
		if (fd > 0) {
			eventfd_write(qfd, fd);
			break;
		}
	}

	free(namelist);
}

int main(int argc, char *argv[])
{
	int qfd = queue_init();
	queue_submit(qfd, gamepad_find);

	while (running) {
		struct pollfd pollfds[] = {
			{ .fd = qfd, .events = POLLIN },
		};

		poll(pollfds, sizeof(pollfds) / sizeof(pollfds[0]), -1);

		if (pollfds[0].revents) {
			uint64_t res;
			int n = read(pollfds[0].fd, &res, sizeof(res));

			printf("Read %d, res = %ld\n", n, res);
		}
	}
}
