#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "entries.h"

struct entry **entries = NULL;
int entries_count = 0;
int entries_capacity = 0;

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

static void append(struct entry *entry)
{
	if (entries_count == entries_capacity) {
		entries_capacity = entries_capacity * 2 + 1;
		entries = reallocarray(entries, entries_capacity, sizeof(struct entry *));
	}

	entries[entries_count++] = entry;
}

void entries_load()
{
	FILE *f = fopen("/home/czak/.config/gameshell/entries", "r");
	if (!f) {
		fprintf(stderr, "No entries file.\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		char *line = read_line(f);
		if (line == NULL) break;

		struct entry *entry = calloc(1, sizeof(struct entry));
		entry->filename = line;

		append(entry);
	}
}
