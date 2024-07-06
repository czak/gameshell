precision mediump float;

#define FONT_PXRANGE 2.0
#define FONT_SIZE 64.0

uniform sampler2D u_Sampler;
uniform vec4 u_Color;
uniform float u_Scale;
uniform float u_ViewportRatio;

varying vec2 v_TexCoord;

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

void main()
{
  vec3 sample = texture2D(u_Sampler, v_TexCoord).rgb;
  float sigDist = median(sample.r, sample.g, sample.b) - 0.5;

  // see:
  // - https://github.com/Chlumsky/msdfgen/blob/master/README.md#using-a-multi-channel-distance-field
  // - https://github.com/Chlumsky/msdfgen/issues/36#issuecomment-429240110
  //
  // note: screenPxRange must never be lower than 1
  float screenPxRange = max(1.0, u_Scale * u_ViewportRatio * FONT_PXRANGE / FONT_SIZE);
  sigDist *= screenPxRange;

  float alpha = clamp(sigDist + 0.5, 0.0, 1.0);
  gl_FragColor = vec4(u_Color.rgb, u_Color.a * alpha);
}
