#pragma once

struct entry {
	char *name;
};

extern struct entry **entries;
extern int entries_count;

void entries_load();
