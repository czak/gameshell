#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "protocols/xdg-shell.h"

const int width = 256;
const int height = 256;

static struct wl_display *wl_display;
static struct wl_registry *wl_registry;

static struct wl_compositor *wl_compositor;
static struct xdg_wm_base *xdg_wm_base;

static struct wl_surface *wl_surface;
static struct xdg_surface *xdg_surface;
static struct xdg_toplevel *xdg_toplevel;

static struct wl_egl_window *wl_egl_window;
static EGLDisplay egl_display;
static EGLContext egl_context;
static EGLSurface egl_surface;

static void noop() {}

static void wl_registry_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version)
{
	// clang-format off
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
	}

	else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		xdg_wm_base = wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1);
	}
	// clang-format on
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = wl_registry_global,
	.global_remove = noop,
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
		uint32_t serial)
{
	xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure,
};

static void init_egl_window()
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

	EGLConfig egl_config = NULL;
	EGLint num_configs = 0;

	egl_display = eglGetDisplay(wl_display);
	eglInitialize(egl_display, NULL, NULL);
	eglChooseConfig(egl_display, config_attributes, &egl_config, 1, &num_configs);

	assert(egl_display && egl_config && num_configs == 1);

	egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attributes);

	assert(egl_context);

	wl_egl_window = wl_egl_window_create(wl_surface, width, height);
	egl_surface = eglCreateWindowSurface(egl_display, egl_config, wl_egl_window, NULL);
	eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
}

int main(int argc, char *argv[])
{
	wl_display = wl_display_connect(NULL);
	wl_registry = wl_display_get_registry(wl_display);
	wl_registry_add_listener(wl_registry, &wl_registry_listener, NULL);
	wl_display_roundtrip(wl_display);

	assert(wl_compositor && xdg_wm_base);

	wl_surface = wl_compositor_create_surface(wl_compositor);
	xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, wl_surface);
	xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

	assert(wl_surface && xdg_surface && xdg_toplevel);

	init_egl_window();

	assert(wl_egl_window && egl_surface);

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	wl_surface_commit(wl_surface);

	fprintf(stderr, "%s\n", glGetString(GL_VERSION));

	while (wl_display_dispatch(wl_display) != -1) {
		glClearColor(0.3f, 0.0f, 0.0f, 0.3f);
		glClear(GL_COLOR_BUFFER_BIT);

		eglSwapBuffers(egl_display, egl_surface);
	}
}
