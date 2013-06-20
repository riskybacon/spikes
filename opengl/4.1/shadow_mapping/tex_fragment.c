#version 150

out vec4 color;

in vec2 fragTC;
in vec4 fragNormal;

uniform sampler2D tex;

void main(void)
{
   color = texture(tex, fragTC).r * vec4(1);
   //   color = vec4(fragTC, 0, 1);
}
