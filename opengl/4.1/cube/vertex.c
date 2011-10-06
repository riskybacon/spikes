#version 150

in vec4 vertex;
in vec4 color;
out vec4 inFragColor;
uniform mat4 mvp;

void main(void)
{
   gl_Position = mvp * vertex;
   inFragColor = color;
}
