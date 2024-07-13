parser = generator(
  find_program('parse_shader.sh'),
  arguments: ['@INPUT@'],
  capture: true,
  output: '@BASENAME@_shader.h',
)

shaders = parser.process(
  'text_frag.glsl',
  'text_vert.glsl',
)
