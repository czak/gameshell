#include <stdlib.h>

#include "menu.h"

void menu_append(struct menu *menu, char *name, void (*callback)(void *data), void *data)
{
	if (menu->items_count == menu->items_capacity) {
		menu->items_capacity = menu->items_capacity * 2 + 1;
		menu->items = reallocarray(menu->items, menu->items_capacity, sizeof(*menu->items));
	}

	struct menu_item *item = &menu->items[menu->items_count];

	// NOTE: name is not copied, potential use after free, e.g. if commands are reloaded
	item->name = name;
	item->callback = callback;
	item->data = data;

	menu->items_count++;
}

void menu_select_next(struct menu *menu)
{
	if (menu->selected_item < menu->items_count - 1) menu->selected_item++;
}

void menu_select_prev(struct menu *menu)
{
	if (menu->selected_item > 0) menu->selected_item--;
}

void menu_trigger(struct menu *menu)
{
	struct menu_item *hover_menu_item = &menu->items[menu->selected_item];
	if (hover_menu_item->callback) {
		hover_menu_item->callback(hover_menu_item->data);
	}
}
