#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <linux/input-event-codes.h>

// I: Bus=0005 Vendor=054c Product=09cc Version=8100
// N: Name="Wireless Controller"
// P: Phys=
// S: Sysfs=/devices/pci0000:00/0000:00:07.1/0000:0f:00.3/usb3/3-4/3-4.3/3-4.3:1.0/bluetooth/hci1/hci1:1/0005:054C:09CC.0009/input/input30
// U: Uniq=a4:ae:12:df:87:54
// H: Handlers=event20 js0
// B: PROP=0
// B: EV=20000b
// B: KEY=7fdb000000000000 0 0 0 0
// B: ABS=3003f
// B: FF=107030000 0

static int parse_handlers(char *line)
{
	char *event = strstr(line, "event");
	if (!event) return -1;

	int n;
	sscanf(event, "event%d", &n);
	return n;
}

static int is_gamepad(char *line)
{
	char *words[12] = {0};
	int num_words = 0;

	line += 7; // after "B: KEY="

	printf("%s", line);

	for (char *word = strtok(line, " \n"); word != NULL; word = strtok(NULL, " \n")) {
		words[num_words++] = word;
	}

	// figure out which word contains the BTN_GAMEPAD bit
	int w = num_words - BTN_GAMEPAD / 64 - 1;
	if (w < 0) {
		printf("no such word\n\n");
		return 0;
	}

	char *word = words[w];

	// figure out which nibble in the word
	int n = strnlen(word, 16) - (BTN_GAMEPAD % 64) / 4 - 1;
	if (n < 0) {
		printf("no such nibble\n\n");
		return 0;
	}

	char nibble = word[n];
	int nibble_num = nibble >= 'a' ? nibble - 87 : nibble - 48;

	printf("%s, nibble '%c' (%x)\n", word, nibble, nibble_num);

	// nibble-sized bitmask
	int bit_window = 1 << (BTN_GAMEPAD % 4);

	return nibble_num & bit_window;
}

int detect()
{
	FILE *f = fopen("/proc/bus/input/devices", "r");
	char line[256];
	int id = -1;

	while (fgets(line, sizeof(line), f)) {
		if (strstr(line, "N: Name=")) {
			id = -1;
		} else if (strstr(line, "H: Handlers=")) {
			id = parse_handlers(line);
		} else if (id != -1 && strstr(line, "B: KEY=")) {
			if (is_gamepad(line)) {
				printf("detected: /dev/input/event%d\n", id);
				// fclose(f);
				// return id;
			}
		}
	}

	fclose(f);
	return -1;
}

int main()
{
	int gamepad = detect();

	if (gamepad == -1) {
		printf("Not detected.\n");
	} else {
		printf("Gamepad detected: /dev/input/event%d\n", gamepad);
	}
}

