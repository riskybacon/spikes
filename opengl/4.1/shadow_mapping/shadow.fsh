#version 150
// Shadow mapping with diffuse lighting

in vec3 N;
in vec2 fragTC;
in vec4 stPos;
in vec4 worldPos;
in vec4 viewLightPos;

uniform sampler2DShadow depthMap;

out vec4 fragColor;

void main(void)
{
   // Default light attenuation factor
   float attenuation = 1.0;

   // The textureProj call does the following:
   // stPos /= stPos.w
   // texture(depthMap, stPos.xy).r
   float occludingDepth = textureProj(depthMap, stPos);
   
   // Shadow mapping happens here - attenuate the light if the distance
   // of the occluding object to the light is less than the distance of
   // the receiver to the light.
   attenuation = occludingDepth < (stPos.z / stPos.w) - 0.0005 ? 0.75 : attenuation;

   // Diffuse color
   vec4 diffuseMaterial = vec4(0.9, 0.6, 0.5, 1.0);

   // Vector from world position to light
   vec3 L = normalize(viewLightPos.xyz - worldPos.xyz);

   // Diffuse color
   vec4 diffuse = diffuseMaterial * max(dot(N,L), 0);

   // Final fragment color
   fragColor = clamp(attenuation * diffuse, 0, 1);
}
