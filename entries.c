#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "entries.h"

static const char *config_path = ".config/gameshell/entries";

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

static char *get_path()
{
	const char *home = getenv("HOME");
	if (!home) {
		return NULL;
	}

	char *path = NULL;
	int ret = asprintf(&path, "%s/%s", home, config_path);
	if (ret == -1) {
		return NULL;
	}

	return path;
}

void entries_load()
{
	char *path = get_path();
	if (!path) {
		fprintf(stderr, "$HOME/%s not found.\n", config_path);
		exit(EXIT_FAILURE);
	}

	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Unable to open %s.\n", path);
		free(path);
		exit(EXIT_FAILURE);
	}

	while (1) {
		char *line = read_line(f);
		if (!line) break;

		struct entry *entry = calloc(1, sizeof(struct entry));
		entry->name = line;

		append(entry);
	}

	free(path);
}
