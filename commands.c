#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "commands.h"

static const char *config_path = ".config/gameshell/commands";

struct command **commands = NULL;
int commands_count = 0;
int commands_capacity = 0;

static char *read_line(FILE *f) {
	char *line = NULL; // NOTE: getline will always allocate a new buffer and we keep it
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

static void append(struct command *command)
{
	if (commands_count == commands_capacity) {
		commands_capacity = commands_capacity * 2 + 1;
		commands = reallocarray(commands, commands_capacity, sizeof(struct command *));
	}

	commands[commands_count++] = command;
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

static void parse_command(char *line, struct command *cmd)
{
	int arg = 0;
	enum { NAME, WDIR, PATH, ARGS } pos = NAME;

	for (char *tok = strtok(line, ";"); tok != NULL; tok = strtok(NULL, ";")) {
		switch (pos) {
			case NAME:
				cmd->name = tok;
				pos = WDIR;
				break;

			case WDIR:
				cmd->wdir = tok;
				pos = PATH;
				break;

			case PATH:
				cmd->path = tok;
				cmd->args[arg++] = tok;
				pos = ARGS;
				break;

			case ARGS:
				assert(arg < MAX_ARGS);
				cmd->args[arg++] = tok;
				break;
		}
	}
}

void commands_load()
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

		struct command *command = calloc(1, sizeof(struct command));
		parse_command(line, command);

		append(command);
	}

	fclose(f);
}

void command_trigger(struct command *cmd)
{
	LOG("trigger: %s, %s, %s", cmd->name, cmd->wdir, cmd->path);
}
