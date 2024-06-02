#pragma once

void window_init(void (*on_draw)(), void (*on_key)(int key));
int window_dispatch();
void window_redraw();
void window_toggle();
