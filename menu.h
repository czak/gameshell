#pragma once

struct menu {
	int hover;
	int selected;
	struct menu_item {
		char *name;
		void (*action)(void *data);
		void *data;
	} items[10];
	int items_count;
};

void menu_select(struct menu *menu);
void menu_deselect(struct menu *menu);
void menu_hover_next_item(struct menu *menu);
void menu_hover_prev_item(struct menu *menu);
void menu_trigger_item(struct menu *menu);
