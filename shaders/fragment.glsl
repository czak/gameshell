precision mediump float;

#define SHADER_ID_IMAGE 0
#define SHADER_ID_SOLID 1

uniform sampler2D u_Sampler;
uniform vec3 u_Color;
uniform int u_ShaderId;

varying vec2 v_TexCoord;

void main()
{
  vec4 texColor = texture2D(u_Sampler, v_TexCoord);

  if (u_ShaderId == SHADER_ID_SOLID) {
    texColor = vec4(u_Color.xyz, texColor.a);
  }

  gl_FragColor = texColor;
}
