#pragma once

struct menu {
	int selected_item;
	struct menu_item {
		char *name;
		void (*action)(void *data);
		void *data;
	} *items;
	int items_count;
	int items_capacity;
};

void menu_append(struct menu *menu, char *name, void (*action)(void *data), void *data);
void menu_select_next_item(struct menu *menu);
void menu_select_prev_item(struct menu *menu);
void menu_trigger_item(struct menu *menu);
