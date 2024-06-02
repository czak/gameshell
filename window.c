#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "protocols/wlr-layer-shell-unstable-v1.h"

#include "window.h"

static struct {
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct wl_compositor *wl_compositor;
	struct wl_seat *wl_seat;
	struct zwlr_layer_shell_v1 *zwlr_layer_shell_v1;
} globals;

static struct {
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
} egl;

static struct {
	struct wl_surface *wl_surface;
	struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1;
	struct wl_egl_window *wl_egl_window;
	EGLSurface egl_surface;

	void (*on_draw)();
	void (*on_key)(int key);

	int visible;
} window;

static const EGLint config_attributes[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
	EGL_NONE,
};

static const EGLint context_attributes[] = {
	EGL_CONTEXT_MAJOR_VERSION, 2,
	EGL_NONE
};

static void noop() {}

static void wl_registry_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version)
{
	// clang-format off
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		globals.wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
	}


	else if (strcmp(interface, wl_seat_interface.name) == 0) {
		globals.wl_seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, 7);
	}

	else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		globals.zwlr_layer_shell_v1 = wl_registry_bind(wl_registry, name, &zwlr_layer_shell_v1_interface, 4);
	}
	// clang-format on
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = wl_registry_global,
	.global_remove = noop,
};

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED) return;

	if (window.on_key)
		window.on_key(key);
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
	.keymap = noop,
	.enter = noop,
	.leave = noop,
	.key = wl_keyboard_key,
	.modifiers = noop,
	.repeat_info = noop,
};

static void zwlr_layer_surface_v1_configure(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1, uint32_t serial,
		uint32_t width, uint32_t height)
{
	zwlr_layer_surface_v1_ack_configure(window.zwlr_layer_surface_v1, serial);

	if (!window.visible) return;

	wl_egl_window_resize(window.wl_egl_window, width, height, 0, 0);
	glViewport(0, 0, width, height);

	window_redraw();
}

static const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_v1_listener = {
	.configure = zwlr_layer_surface_v1_configure,
	.closed = noop,
};

void window_init(void (*on_draw)(), void (*on_key)(int key))
{
	globals.wl_display = wl_display_connect(NULL);
	globals.wl_registry = wl_display_get_registry(globals.wl_display);
	wl_registry_add_listener(globals.wl_registry, &wl_registry_listener, NULL);
	wl_display_roundtrip(globals.wl_display);

	assert(globals.wl_compositor && globals.wl_seat && globals.zwlr_layer_shell_v1);

	window.wl_surface = wl_compositor_create_surface(globals.wl_compositor);
	window.zwlr_layer_surface_v1 = zwlr_layer_shell_v1_get_layer_surface(
			globals.zwlr_layer_shell_v1, window.wl_surface, NULL,
			ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "gameshell");

	assert(window.wl_surface && window.zwlr_layer_surface_v1);

	egl.display = eglGetDisplay(globals.wl_display);
	eglInitialize(egl.display, NULL, NULL);

	EGLint num_configs = 0;
	eglChooseConfig(egl.display, config_attributes, &egl.config, 1, &num_configs);

	assert(egl.display && egl.config && num_configs == 1);

	egl.context = eglCreateContext(egl.display, egl.config, EGL_NO_CONTEXT, context_attributes);

	assert(egl.context);

	window.wl_egl_window = wl_egl_window_create(window.wl_surface, 200, 200);
	window.egl_surface = eglCreateWindowSurface(egl.display, egl.config, window.wl_egl_window, NULL);
	eglMakeCurrent(egl.display, window.egl_surface, window.egl_surface, egl.context);

	assert(window.wl_egl_window && window.egl_surface);

	zwlr_layer_surface_v1_add_listener(window.zwlr_layer_surface_v1, &zwlr_layer_surface_v1_listener, NULL);
	zwlr_layer_surface_v1_set_exclusive_zone(window.zwlr_layer_surface_v1, -1);
	zwlr_layer_surface_v1_set_keyboard_interactivity(window.zwlr_layer_surface_v1,
			ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);
	wl_surface_commit(window.wl_surface);

	struct wl_keyboard *wl_keyboard = wl_seat_get_keyboard(globals.wl_seat);
	wl_keyboard_add_listener(wl_keyboard, &wl_keyboard_listener, NULL);

	window.on_draw = on_draw;
	window.on_key = on_key;
	window.visible = 1;
}

int window_dispatch()
{
	return wl_display_dispatch(globals.wl_display);
}

void window_redraw()
{
	if (window.on_draw)
		window.on_draw();

	eglSwapBuffers(egl.display, window.egl_surface);
}

void window_toggle()
{
	window.visible = 0;

	wl_surface_attach(window.wl_surface, NULL, 0, 0);
	wl_surface_commit(window.wl_surface);
	window_dispatch();

	sleep(1);

	window.visible = 1;

	eglSwapBuffers(egl.display, window.egl_surface);
	window_dispatch();
}
