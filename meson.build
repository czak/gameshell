project('gameshell', 'c')

add_project_arguments('-D_GNU_SOURCE', language: 'c')

if get_option('buildtype').startswith('debug')
  add_project_arguments('-D_LOG_ENABLE', language: 'c')
endif

protocols = declare_dependency(
  sources: [
    'protocols/xdg-shell.c',
  ],
  include_directories: ['protocols'],
)

wayland = dependency('wayland-client')

executable(
  'gameshell',
  'main.c',
  dependencies: [
    protocols,
    wayland,
  ],
)
