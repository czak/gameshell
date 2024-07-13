precision mediump float;

attribute vec2 a_Position;

uniform vec2 u_Viewport;

void main()
{
	gl_Position = vec4(
		2.0 * a_Position.x / u_Viewport.x - 1.0,
		1.0 - 2.0 * a_Position.y / u_Viewport.y,
		0.0,
		1.0);
}
