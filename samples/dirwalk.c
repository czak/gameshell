#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void list_directory_contents(const char *path)
{
	struct dirent **namelist;

	int n = scandir(path, &namelist, NULL, alphasort);
	if (n < 0) {
		perror("scandir");
		return;
	}

	printf("Contents of directory %s:\n", path);
	for (int i = 0; i < n; i++) {
		printf("%s\n", namelist[i]->d_name);
		free(namelist[i]);
	}
	free(namelist);
}

int main()
{
	char current_path[PATH_MAX];
	char new_path[PATH_MAX];

	while (1) {
		if (getcwd(current_path, sizeof(current_path)) == NULL) {
			perror("getcwd");
			return EXIT_FAILURE;
		}

		list_directory_contents(current_path);

		printf("\nEnter directory to change to (or 'exit' to quit): ");
		if (fgets(new_path, sizeof(new_path), stdin) == NULL) {
			perror("fgets");
			continue;
		}

		// Remove newline character
		new_path[strcspn(new_path, "\n")] = '\0';

		if (strcmp(new_path, "exit") == 0) {
			break;
		}

		if (chdir(new_path) != 0) {
			perror("chdir");
			continue;
		}
	}

	return EXIT_SUCCESS;
}
