#include <assert.h>
#include <stdlib.h>

#include "menu.h"

struct menu_item menu_get_menu_item(struct menu *menu, int index) {
	assert(index >= 0 && index < menu->items_count);

	return menu->resolver(menu->items[index]);
}

void menu_append(struct menu *menu, void *item)
{
	if (menu->items_count == menu->items_capacity) {
		menu->items_capacity = menu->items_capacity * 2 + 1;
		menu->items = reallocarray(menu->items, menu->items_capacity, sizeof(*menu->items));
	}

	menu->items[menu->items_count++] = item;
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
	struct menu_item menu_item = menu_get_menu_item(menu, menu->selected_item);
	if (menu_item.callback) {
		menu_item.callback(menu_item.item);
	}
}
