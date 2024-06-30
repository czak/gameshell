#include <stdlib.h>

#include "menu.h"

void menu_append(struct menu *menu, char *name, void (*action)(void *data), void *data)
{
	if (menu->items_count == menu->items_capacity) {
		menu->items_capacity = menu->items_capacity * 2 + 1;
		menu->items = reallocarray(menu->items, menu->items_capacity, sizeof(struct menu_item));
	}

	struct menu_item *item = &menu->items[menu->items_count];

	item->name = name;
	item->action = action;
	item->data = data;

	menu->items_count++;
}

void menu_select(struct menu *menu)
{
	menu->selected = 1;
}

void menu_deselect(struct menu *menu)
{
	menu->selected = 0;
}

void menu_hover_next_item(struct menu *menu)
{
	if (menu->hover < menu->items_count - 1) menu->hover++;
}

void menu_hover_prev_item(struct menu *menu)
{
	if (menu->hover > 0) menu->hover--;
}

void menu_trigger_item(struct menu *menu)
{
	struct menu_item *hover_menu_item = &menu->items[menu->hover];
	if (hover_menu_item->action) {
		hover_menu_item->action(hover_menu_item->data);
	}
}
