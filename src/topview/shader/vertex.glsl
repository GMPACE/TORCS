#version 120    // GLSL 1.2  (OpenGL 2.x)

uniform mat4 u_P;
uniform mat4 u_V;
uniform mat4 u_M;

attribute vec4 a_position;
attribute vec4 a_color;
varying vec4 v_color;


void main()
{
  v_color = a_color;
  gl_Position = u_P * u_V * u_M * a_position;
}
