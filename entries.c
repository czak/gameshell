#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "entries.h"

#define MAX_ENTRIES 30

struct entry *entries[MAX_ENTRIES] = {0};
int entries_count = 0;

static char *read_line(FILE *f)
{
	char *line = NULL;
	size_t n;
	ssize_t res = getline(&line, &n, f);
	if (res == -1) {
		return NULL;
	}
	if (line[res-1] == '\n') {
		line[res-1] = '\0';
	}
	return line;
}

void entries_load()
{
	FILE *f = fopen("/home/czak/.config/gameshell/entries", "r");
	if (!f) {
		fprintf(stderr, "No entries file.\n");
		exit(EXIT_FAILURE);
	}

	while (entries_count < MAX_ENTRIES) {
		char *line = read_line(f);
		if (line == NULL) break;

		struct entry *entry = calloc(1, sizeof(struct entry));
		entry->filename = line;

		entries[entries_count++] = entry;
	}
}
