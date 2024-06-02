#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "protocols/wlr-layer-shell-unstable-v1.h"

static struct wl_display *wl_display;
static struct wl_registry *wl_registry;
static struct wl_compositor *wl_compositor;
static struct zwlr_layer_shell_v1 *zwlr_layer_shell_v1;

static struct wl_surface *wl_surface;
static struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1;

static struct wl_egl_window *wl_egl_window;
static EGLDisplay egl_display;
static EGLConfig egl_config;
static EGLContext egl_context;
static EGLSurface egl_surface;

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
		wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
	}

	else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		zwlr_layer_shell_v1 = wl_registry_bind(wl_registry, name, &zwlr_layer_shell_v1_interface, 4);
	}
	// clang-format on
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = wl_registry_global,
	.global_remove = noop,
};

static void zwlr_layer_surface_v1_configure(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1, uint32_t serial,
		uint32_t width, uint32_t height)
{
	zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);

	wl_egl_window_resize(wl_egl_window, width, height, 0, 0);
	glViewport(0, 0, width, height);
}

static const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_v1_listener = {
	.configure = zwlr_layer_surface_v1_configure,
	.closed = noop,
};

void window_init()
{
	wl_display = wl_display_connect(NULL);
	wl_registry = wl_display_get_registry(wl_display);
	wl_registry_add_listener(wl_registry, &wl_registry_listener, NULL);
	wl_display_roundtrip(wl_display);

	assert(wl_compositor && zwlr_layer_shell_v1);

	wl_surface = wl_compositor_create_surface(wl_compositor);
	zwlr_layer_surface_v1 = zwlr_layer_shell_v1_get_layer_surface(
			zwlr_layer_shell_v1, wl_surface, NULL,
			ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "gameshell");

	assert(wl_surface && zwlr_layer_surface_v1);

	egl_display = eglGetDisplay(wl_display);
	eglInitialize(egl_display, NULL, NULL);

	EGLint num_configs = 0;
	eglChooseConfig(egl_display, config_attributes, &egl_config, 1, &num_configs);

	assert(egl_display && egl_config && num_configs == 1);

	egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attributes);

	assert(egl_context);

	wl_egl_window = wl_egl_window_create(wl_surface, 200, 200);
	egl_surface = eglCreateWindowSurface(egl_display, egl_config, wl_egl_window, NULL);
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

	assert(wl_egl_window && egl_surface);

	zwlr_layer_surface_v1_add_listener(zwlr_layer_surface_v1, &zwlr_layer_surface_v1_listener, NULL);
	zwlr_layer_surface_v1_set_exclusive_zone(zwlr_layer_surface_v1, -1);
	wl_surface_commit(wl_surface);
}

int window_dispatch()
{
	return wl_display_dispatch(wl_display);
}

void window_swap()
{
	eglSwapBuffers(egl_display, egl_surface);
}
