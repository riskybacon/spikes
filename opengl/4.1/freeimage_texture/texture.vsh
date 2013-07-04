#version 150

in vec4 vertex;
in vec2 tc;

out vec2 fragTC;

uniform mat4 mvp;

void main(void)
{
   // Transform vertex into view volume
   gl_Position = mvp * vertex;
 
   fragTC = tc;
}
