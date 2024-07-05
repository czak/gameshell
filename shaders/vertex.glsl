precision mediump float;

attribute vec2 a_Position;
attribute vec2 a_TexCoord;

uniform vec2 u_Position;
uniform vec2 u_Offset;
uniform vec2 u_Viewport;
uniform float u_Scale;

varying vec2 v_TexCoord;

void main()
{
	vec2 pos = u_Scale * (a_Position + u_Position) + u_Offset;
	gl_Position = vec4(
		2.0 * pos.x / u_Viewport.x - 1.0,
		1.0 - 2.0 * pos.y / u_Viewport.y,
		0.0,
		1.0);
	v_TexCoord = a_TexCoord;
}
