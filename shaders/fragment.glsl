precision mediump float;

uniform sampler2D u_Sampler;
uniform vec4 u_Color;
uniform float u_Scale;

varying vec2 v_TexCoord;

void main()
{
  vec2 st = floor(v_TexCoord) + 0.5;
  float alpha = texture2D(u_Sampler, st / 128.0).a;
  gl_FragColor = vec4(u_Color.rgb, u_Color.a * alpha);
}
