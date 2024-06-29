#pragma once

#include <stdint.h>

void signals_init(void (*on_child)(uint32_t pid, int32_t code));
int signals_get_fd();
void signals_dispatch();
