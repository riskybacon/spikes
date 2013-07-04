#version 150

in vec2 fragTC;
out vec4 color;
uniform sampler2D tex;

void main(void)
{
   color = texture(tex, fragTC);
}
