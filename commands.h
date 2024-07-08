#pragma once

#include <sys/types.h>

#define MAX_ARGS 10

struct command {
	char *name;
	char *wdir;
	char *path;
	char *args[MAX_ARGS];
	pid_t pid;
	int stopped;
};

extern struct command *commands;
extern int commands_count;

void commands_load();
void command_exec(struct command *cmd);
struct command *command_find(pid_t pid);
