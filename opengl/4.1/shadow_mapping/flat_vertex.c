#version 150

in vec4 vertex;
uniform mat4 mvp;

void main(void)
{
   // Transform vertex into view volume
   gl_Position = mvp * vertex;
}
