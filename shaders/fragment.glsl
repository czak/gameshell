precision mediump float;

#define DISTANCE_RANGE 4.0 // value used for msdf-bmfont -r argument

uniform sampler2D u_Sampler;
uniform vec3 u_Color;
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
  sigDist *= DISTANCE_RANGE * u_Scale;

  float alpha = clamp(sigDist + 0.5, 0.0, 1.0);
  gl_FragColor = vec4(u_Color.rgb, alpha);
}
