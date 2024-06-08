precision mediump float;

uniform sampler2D u_Sampler;
uniform vec3 u_Color;

varying vec2 v_TexCoord;

void main()
{
  float a = texture2D(u_Sampler, v_TexCoord).a;
  gl_FragColor = vec4(u_Color.xyz * a, a);
}
