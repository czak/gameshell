precision mediump float;

#define FONT_PXRANGE 2.0
#define FONT_SIZE 64.0

uniform sampler2D u_Sampler;
uniform vec4 u_Color;
uniform float u_Scale;

varying vec2 v_TexCoord;

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

void main()
{
  vec3 sample = texture2D(u_Sampler, v_TexCoord).rgb;
  float sigDist = median(sample.r, sample.g, sample.b) - 0.5;

  // see https://github.com/Chlumsky/msdfgen/issues/36#issuecomment-429240110
  sigDist *= u_Scale * FONT_PXRANGE / FONT_SIZE;

  float alpha = clamp(sigDist + 0.5, 0.0, 1.0);
  gl_FragColor = vec4(u_Color.rgb, u_Color.a * alpha);
}
