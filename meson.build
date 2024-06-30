project('gameshell', 'c')

add_project_arguments('-D_GNU_SOURCE', language: 'c')

if get_option('buildtype').startswith('debug')
  add_project_arguments('-D_LOG_ENABLE', language: 'c')
endif

subdir('shaders')

protocols = declare_dependency(
  sources: [
    'protocols/xdg-shell.c',
    'protocols/wlr-layer-shell-unstable-v1.c',
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
  'fonts/jetbrains.c',
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
