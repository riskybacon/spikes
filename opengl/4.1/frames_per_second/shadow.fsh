#version 150
// Shadow mapping with diffuse lighting

in vec2 fragTC;
in vec4 stPos;

uniform sampler2DShadow depthMap;

out vec4 fragColor;

void main(void)
{
   // Default light attenuation factor
   float shadowFactor = textureProj(depthMap, stPos);
   
   // Diffuse color
   vec4 diffuse = vec4(0.25, 0.75, 0.75, 1.0);

   // Final fragment color
   //   fragColor = clamp(attenuation * diffuse, 0, 1);
   fragColor = vec4(shadowFactor);
}