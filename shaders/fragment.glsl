precision mediump float;

uniform sampler2D u_Sampler;
uniform vec3 u_Color;

varying vec2 v_TexCoord;

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

void main()
{
  vec3 sample = texture2D(u_Sampler, v_TexCoord).rgb;
  float sigDist = median(sample.r, sample.g, sample.b) - 0.5;

  // TODO: move to distanceFactor uniform
  // see https://github.com/Chlumsky/msdfgen/issues/36#issuecomment-429240110
  sigDist *= 4.0;

  float alpha = clamp(sigDist + 0.5, 0.0, 1.0);
  gl_FragColor = vec4(u_Color.rgb, alpha);
}
