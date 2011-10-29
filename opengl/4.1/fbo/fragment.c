#version 150

in vec4 fragColor;
in vec2 fragTC;

out vec4 color;

uniform sampler2D tex;

void main(void)
{
   color = texture(tex, fragTC);
//   color.r *= fragTC.x;
//   color.b *= fragTC.y;
}
