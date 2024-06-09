#include <stddef.h>

#include "entries.h"

struct entry *entries[] = {
	&(struct entry){ "Ghostrunner", "/home/czak/projects/gameshell/boxart/ghostrunner.png" },
	&(struct entry){ "GRID", "/home/czak/projects/gameshell/boxart/grid.png" },
	&(struct entry){ "Virtua Tennis 3", "/home/czak/projects/gameshell/boxart/virtuatennis3.png" },
	&(struct entry){ "Witcher 3", "/home/czak/projects/gameshell/boxart/witcher3.png" },
	&(struct entry){ "Wolfenstein: The New Colossus", "/home/czak/projects/gameshell/boxart/wolfenstein.png" },
	NULL,
};

int entries_count = 5;

void entries_load()
{
}
