precision mediump float;

uniform sampler2D u_Sampler;
uniform vec4 u_Color;
uniform float u_ScreenPxRange;

varying vec2 v_TexCoord;

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

float opacity(vec2 uv) {
  vec3 sample = texture2D(u_Sampler, uv).rgb;
  float sd = median(sample.r, sample.g, sample.b) - 0.5;
  return clamp(sd * u_ScreenPxRange + 0.5, 0.0, 1.0);
}

void main()
{
  float fg_alpha = opacity(v_TexCoord);
  float shadow_alpha = opacity(v_TexCoord - 2.0 / 512.0);

  vec4 fg = vec4(u_Color.rgb, u_Color.a * fg_alpha);
  vec4 shadow = vec4(0.0, 0.0, 0.0, shadow_alpha);

  gl_FragColor = mix(shadow, fg, fg_alpha);
}
