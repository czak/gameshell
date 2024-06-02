#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

#include "protocols/xdg-shell.h"

static struct wl_display *wl_display;
static struct wl_registry *wl_registry;

static struct wl_shm *wl_shm;
static struct wl_compositor *wl_compositor;
static struct xdg_wm_base *xdg_wm_base;

static struct wl_surface *wl_surface;
static struct xdg_surface *xdg_surface;
static struct xdg_toplevel *xdg_toplevel;

static struct wl_buffer *wl_buffer;

static void noop() {}

static void wl_registry_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version)
{
	// clang-format off
	if (strcmp(interface, wl_shm_interface.name) == 0) {
		wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
	}

	else if (strcmp(interface, wl_compositor_interface.name) == 0) {
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

	wl_surface_attach(wl_surface, wl_buffer, 0, 0);
	wl_surface_commit(wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure,
};

void init_buffer()
{
	const int width = 256;
	const int height = 256;

	const int stride = width * sizeof(uint32_t); // 4 bytes per
	const int size = stride * height;

	int fd = memfd_create("buffer-pool", 0);
	ftruncate(fd, size);

	uint32_t *pixels = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			pixels[width * y + x] = 0x3f00ff3f;
		}
	}
	munmap(pixels, size);

	struct wl_shm_pool *wl_shm_pool = wl_shm_create_pool(wl_shm, fd, size);
	wl_buffer = wl_shm_pool_create_buffer(wl_shm_pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
	wl_shm_pool_destroy(wl_shm_pool);

	close(fd);
}

int main(int argc, char *argv[])
{
	wl_display = wl_display_connect(NULL);
	wl_registry = wl_display_get_registry(wl_display);
	wl_registry_add_listener(wl_registry, &wl_registry_listener, NULL);
	wl_display_roundtrip(wl_display);

	assert(wl_shm && wl_compositor && xdg_wm_base);

	wl_surface = wl_compositor_create_surface(wl_compositor);
	xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, wl_surface);
	xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

	assert(wl_surface && xdg_surface && xdg_toplevel);

	init_buffer();

	assert(wl_buffer);

	xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	wl_surface_commit(wl_surface);

	while (wl_display_dispatch(wl_display) != -1) {
	}
}
