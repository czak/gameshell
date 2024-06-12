#pragma once

void window_init(void (*on_draw)(), void (*on_resize)(int width, int height), void (*on_key)(int key));
int window_get_fd();
void window_flush();
int window_dispatch();
void window_redraw();
int window_visible();
void window_toggle();
