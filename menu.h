#pragma once

struct menu_item {
	char *name;
	void (*callback)(void *item);
	void *item;
};

struct menu {
	int selected_item;
	void **items;
	int items_count;
	int items_capacity;

	// Turns opaque "item" into a struct menu_item
	// Each menu needs to provide its own resolver
	struct menu_item (*resolver)(void *item);
};

void menu_append(struct menu *menu, void *item);
void menu_select_next(struct menu *menu);
void menu_select_prev(struct menu *menu);
void menu_trigger(struct menu *menu);
