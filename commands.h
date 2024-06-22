#pragma once

#include <sys/types.h>

#define MAX_ARGS 10

struct command {
	char *name;
	char *wdir;
	char *path;
	char *args[MAX_ARGS];
	pid_t pid;
};

extern struct command **commands;
extern int commands_count;

void commands_load();
void command_trigger(struct command *cmd);
