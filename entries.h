#pragma once

struct entry {
	char *name;
	char *filename;
	int image;
};

extern struct entry **entries;
extern int entries_count;

void entries_load();
