precision mediump float;

uniform sampler2D u_Sampler;

varying vec2 v_TexCoord;

void main()
{
  float a = texture2D(u_Sampler, v_TexCoord).a;
  gl_FragColor = vec4(1.0 * a, 0.75 * a, 0.3 * a, a);
}
