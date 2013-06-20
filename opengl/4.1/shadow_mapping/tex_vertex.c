#version 150

in vec4 vertex;
in vec4 normal;
in vec2 tc;

uniform mat4 mvp;

out vec2 fragTC;
out vec4 fragNormal;

void main(void)
{
   // Transform vertex into view volume
   gl_Position = mvp * vertex;
   fragTC = tc;
   fragNormal = normal;
}
