#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "commands.h"

static const char *config_path = ".config/gameshell/commands";

struct command *commands = NULL;
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

static void append(struct command command)
{
	if (commands_count == commands_capacity) {
		commands_capacity = commands_capacity * 2 + 1;
		commands = reallocarray(commands, commands_capacity, sizeof(struct command));
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

static struct command parse(char *line)
{
	struct command command = {0};

	int arg = 0;
	enum { NAME, WDIR, PATH, ARGS } pos = NAME;

	for (char *tok = strtok(line, ";"); tok != NULL; tok = strtok(NULL, ";")) {
		switch (pos) {
			case NAME:
				command.name = tok;
				pos = WDIR;
				break;

			case WDIR:
				command.wdir = tok;
				pos = PATH;
				break;

			case PATH:
				command.path = tok;
				command.args[arg++] = tok; // argv[0] for convenience
				pos = ARGS;
				break;

			case ARGS:
				assert(arg < MAX_ARGS);
				command.args[arg++] = tok;
				break;
		}
	}

	return command;
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

		append(parse(line));
	}

	fclose(f);
}

void command_exec(struct command *command)
{
	pid_t pid = fork();

	if (pid > 0) {
		command->pid = pid;
    } else if (pid == 0) {
		setpgid(0, 0);
		if (command->wdir) {
			chdir(command->wdir);
		}
        execvp(command->path, command->args);

		// exec only returns if an error occured
        LOG("Exec in child %d failed", getpid());
		exit(EXIT_FAILURE);
    } else {
        LOG("Fork failed");
	}
}

struct command *command_find(pid_t pid) {
	for (int i = 0; i < commands_count; i++) {
		if (commands[i].pid == pid) {
			return &commands[i];
		}
	}

	return NULL;
}
