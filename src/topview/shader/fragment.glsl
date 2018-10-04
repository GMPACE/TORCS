#version 120
uniform sampler2D u_texid;
uniform int u_tex_flag;
varying vec2 v_texcoord;
varying vec4 v_color;

void main()
{
  vec4 color = texture2D(u_texid, v_texcoord);
  gl_FragColor = (1 - u_tex_flag) * v_color + u_tex_flag * color;
//  gl_FragColor = v_color;
}
