#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "protocols/wlr-layer-shell-unstable-v1.h"
#include "protocols/fractional-scale-v1.h"
#include "protocols/viewporter.h"

#include "log.h"
#include "window.h"

static struct {
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct wl_compositor *wl_compositor;
	struct wl_seat *wl_seat;
	struct zwlr_layer_shell_v1 *zwlr_layer_shell_v1;
	struct wp_fractional_scale_manager_v1 *wp_fractional_scale_manager_v1;
	struct wp_viewporter *wp_viewporter;
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
	void (*on_resize)(int width, int height);
	void (*on_key)(int key);

	int width, height;
	int visible;
} window;

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

	else if (strcmp(interface, wp_fractional_scale_manager_v1_interface.name) == 0) {
		globals.wp_fractional_scale_manager_v1 = wl_registry_bind(wl_registry, name, &wp_fractional_scale_manager_v1_interface, 1);
	}

	else if (strcmp(interface, wp_viewporter_interface.name) == 0) {
		globals.wp_viewporter = wl_registry_bind(wl_registry, name, &wp_viewporter_interface, 1);
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

	window.visible = 1;

	if (width > 0) window.width = width;
	if (height > 0) window.height = height;

	wl_egl_window_resize(window.wl_egl_window, window.width, window.height, 0, 0);

	if (window.on_resize)
		window.on_resize(window.width, window.height);

	// HACK: Additional swap so the EGL surface definitely gets resized.
	//       - Sway sends 2 configure events so this is redundant
	//       - But Hyprland & KWin send only 1 and first frame may swap old size
	//         surface
	eglSwapBuffers(egl.display, window.egl_surface);

	window_redraw();
}

static void zwlr_layer_surface_v1_closed(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1)
{
	log_error("Unexpected layer closed");
}

static const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_v1_listener = {
	.configure = zwlr_layer_surface_v1_configure,
	.closed = zwlr_layer_surface_v1_closed,
};

static void egl_init()
{
	const EGLint config_attributes[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_NONE,
	};

	const EGLint context_attributes[] = {
		EGL_CONTEXT_MAJOR_VERSION, 2,
		EGL_NONE
	};

	EGLint num_configs = 0;

	egl.display = eglGetDisplay(globals.wl_display);
	eglInitialize(egl.display, NULL, NULL);
	eglChooseConfig(egl.display, config_attributes, &egl.config, 1, &num_configs);

	if (!egl.display) log_fatal("eglGetDisplay failed");
	if (!egl.config || num_configs != 1) log_fatal("eglChooseConfig failed");

	egl.context = eglCreateContext(egl.display, egl.config, EGL_NO_CONTEXT, context_attributes);

	if (!egl.context) log_fatal("Failed to create EGL rendering context");

	window.wl_egl_window = wl_egl_window_create(window.wl_surface, 200, 200);
	window.egl_surface = eglCreateWindowSurface(egl.display, egl.config, window.wl_egl_window, NULL);

	if (!window.egl_surface) log_fatal("Failed to create EGL rendering surface");

	eglMakeCurrent(egl.display, window.egl_surface, window.egl_surface, egl.context);
}

void window_init(void (*on_draw)(), void (*on_resize)(int width, int height), void (*on_key)(int key))
{
	globals.wl_display = wl_display_connect(NULL);
	globals.wl_registry = wl_display_get_registry(globals.wl_display);
	wl_registry_add_listener(globals.wl_registry, &wl_registry_listener, NULL);
	wl_display_roundtrip(globals.wl_display);

	if (!globals.wl_compositor) log_fatal("Failed to bind wl_compositor");
	if (!globals.wl_seat) log_fatal("Failed to bind wl_seat");
	if (!globals.zwlr_layer_shell_v1) log_fatal("Failed to bind zwlr_layer_shell_v1");
	if (!globals.wp_fractional_scale_manager_v1) log_fatal("Failed to bind wp_fractional_scale_manager_v1");
	if (!globals.wp_viewporter) log_fatal("Failed to bind wp_viewporter");

	window.wl_surface = wl_compositor_create_surface(globals.wl_compositor);

	if (!window.wl_surface) log_fatal("Failed to create surface");

	window.width = 1920;
	window.height = 1080;
	window.on_draw = on_draw;
	window.on_resize = on_resize;
	window.on_key = on_key;

	egl_init();
	window_show();
}

void window_show()
{
	window.zwlr_layer_surface_v1 = zwlr_layer_shell_v1_get_layer_surface(
			globals.zwlr_layer_shell_v1, window.wl_surface, NULL,
			ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "gameshell");

	if (!window.zwlr_layer_surface_v1) log_fatal("Failed to create layer surface");

	zwlr_layer_surface_v1_add_listener(window.zwlr_layer_surface_v1, &zwlr_layer_surface_v1_listener, NULL);
	zwlr_layer_surface_v1_set_anchor(window.zwlr_layer_surface_v1,
			ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP + ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM +
			ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT + ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
	zwlr_layer_surface_v1_set_exclusive_zone(window.zwlr_layer_surface_v1, -1);

	if (window.on_key) {
		zwlr_layer_surface_v1_set_keyboard_interactivity(
				window.zwlr_layer_surface_v1,
				ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);

		struct wl_keyboard *wl_keyboard = wl_seat_get_keyboard(globals.wl_seat);
		wl_keyboard_add_listener(wl_keyboard, &wl_keyboard_listener, NULL);
	}

	wl_surface_commit(window.wl_surface);
}

void window_hide()
{
	zwlr_layer_surface_v1_destroy(window.zwlr_layer_surface_v1);
	wl_surface_attach(window.wl_surface, NULL, 0, 0);
	wl_surface_commit(window.wl_surface);

	window.visible = 0;
}

int window_get_fd()
{
	return wl_display_get_fd(globals.wl_display);
}

void window_flush()
{
	wl_display_flush(globals.wl_display);
}

int window_dispatch()
{
	return wl_display_dispatch(globals.wl_display);
}

void window_redraw()
{
	if (!window.visible) return;

	if (window.on_draw)
		window.on_draw();

	eglSwapBuffers(egl.display, window.egl_surface);
}

int window_visible()
{
	return window.visible;
}
