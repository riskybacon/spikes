#version 150

in vec4 fragColor;
in vec2 fragTC;

out vec4 color;

uniform sampler2D tex;

void main(void)
{
  vec4 sample = texture(tex, fragTC);
  color = sample;
  //color = vec4(fragTC, 0, 1);
}
