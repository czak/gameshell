attribute vec2 a_Position;
attribute vec2 a_TexCoord;

uniform vec2 u_Offset;
varying vec2 v_TexCoord;

void main()
{
  float xs = 0.001041666666666666666666667; // 2 / 1920
  float ys = 0.001851851851851851851851852; // 2 / 1080
  gl_Position = vec4(xs * (a_Position.x + u_Offset.x) - 1.0, 1.0 - ys * (a_Position.y + u_Offset.y), 0.0, 1.0);
  v_TexCoord = a_TexCoord / 256.0;
}
