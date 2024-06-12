#pragma once

/// Button codes from linux/input-event-codes.h
#define BTN_DPAD_UP		0x220
#define BTN_DPAD_DOWN	0x221
#define BTN_DPAD_LEFT	0x222
#define BTN_DPAD_RIGHT	0x223

#define BTN_GAMEPAD		0x130
#define BTN_SOUTH		0x130
#define BTN_EAST		0x131
#define BTN_NORTH		0x133
#define BTN_WEST		0x134
#define BTN_TL			0x136
#define BTN_TR			0x137
#define BTN_TL2			0x138
#define BTN_TR2			0x139
#define BTN_SELECT		0x13a
#define BTN_START		0x13b
#define BTN_MODE		0x13c
#define BTN_THUMBL		0x13d
#define BTN_THUMBR		0x13e

#define GAMEPAD_UNGRABBED	0
#define GAMEPAD_GRABBED		1

void gamepad_init(int grab, void (*on_button)(int button));
int gamepad_get_inotify();
int gamepad_get_fd();
void gamepad_dispatch();
void gamepad_grab();
void gamepad_ungrab();
