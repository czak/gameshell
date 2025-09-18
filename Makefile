CC = gcc
CFLAGS = -D_GNU_SOURCE $(shell pkg-config --cflags wayland-client wayland-egl egl glesv2)
LDFLAGS = -lm $(shell pkg-config --libs wayland-client wayland-egl egl glesv2)

SOURCES = gameshell.c \
					commands.c \
					gamepad.c \
					gfx.c \
					image.c \
					log.c \
					menu.c \
					signals.c \
					system.c \
					window.c

PROTOCOLS = protocols/xdg-shell.c \
						protocols/wlr-layer-shell-unstable-v1.c \
						protocols/fractional-scale-v1.c \
						protocols/viewporter.c

FONTS = fonts/chakra.c

SHADERS = shaders/text_frag.glsl \
					shaders/text_vert.glsl \
					shaders/rect_frag.glsl \
					shaders/rect_vert.glsl

OBJECTS = $(SOURCES:.c=.o) $(PROTOCOLS:.c=.o) $(FONTS:.c=.o)
SHADER_HEADERS = $(SHADERS:.glsl=.h)

gameshell: $(OBJECTS)

gfx.o: gfx.c $(SHADER_HEADERS)

shaders/%.h: shaders/%.glsl
	shaders/parse_shader.sh $< > $@

.PHONY: clean
clean:
	rm -f gameshell $(OBJECTS) $(SHADER_HEADERS)
