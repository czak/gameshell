#include "menu.h"

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
