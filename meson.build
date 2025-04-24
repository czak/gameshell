project('gameshell', 'c', default_options: ['b_ndebug=if-release'])

add_project_arguments('-D_GNU_SOURCE', language: 'c')

subdir('shaders')

protocols = declare_dependency(
  sources: [
    'protocols/xdg-shell.c',
    'protocols/wlr-layer-shell-unstable-v1.c',
    'protocols/fractional-scale-v1.c',
    'protocols/viewporter.c',
  ],
  include_directories: ['protocols'],
)

math = meson.get_compiler('c').find_library('m')
wayland = dependency('wayland-client')
wayland_egl = dependency('wayland-egl')
egl = dependency('egl')
glesv2 = dependency('glesv2')

executable(
  'gameshell',
  'main.c',
  'window.c',
  'image.c',
  'gfx.c',
  'commands.c',
  'log.c',
  'gamepad.c',
  'menu.c',
  'signals.c',
  'system.c',
  'fonts/chakra.c',
  shaders,
  dependencies: [
    math,
    protocols,
    wayland,
    wayland_egl,
    egl,
    glesv2,
  ],
)
