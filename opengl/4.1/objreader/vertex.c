#version 150

in vec4 vertex;
in vec4 normal;
in vec2 tc;

out vec4 inFragColor;
uniform mat4 mvp;

void main(void)
{
   gl_Position = mvp * vertex;
   inFragColor = normal;
}
