#version 150
// Shadow mapping with diffuse lighting

in vec2 fragTC;
in vec4 stPos;

uniform sampler2DShadow depthMap;
uniform vec2 texmapScale;

out vec4 fragColor;

#if 1

float offsetLookup(sampler2DShadow map, vec4 loc, vec2 offset)
{
   return textureProj(map, vec4(loc.xy + offset * texmapScale * loc.w, loc.z, loc.w));
}

#if 1
float pcf(sampler2DShadow map, vec4 loc)
{
   float sum = 0;
   float x, y;
   int numSamples = 0;
   for (y = -1.5; y <= 1.5; y += 1.0)
   {
      for (x = -1.5; x <= 1.5; x += 1.0)
      {
         sum += offsetLookup(map, loc, vec2(x, y));
         numSamples++;
      }
   }
   
   return sum / numSamples;
   
}
#else

float pcf(sampler2DShadow map, vec4 loc)
{
   vec2 offset = fract(loc.xy * 0.5);
   
   offset.x = offset.x > 0.25 ? 1.0 : 0.0;
   offset.y = offset.y > 0.25 ? 1.0 : 0.0;
   
   offset.y += offset.x;  // y ^= x in floating point

   if (offset.y > 1.1)
   {
      offset.y = 0;
   }
   
   float shadowCoeff = (offsetLookup(map, loc, offset + vec2(-1.5,  0.5)) +
                        offsetLookup(map, loc, offset + vec2( 0.5,  0.5)) +
                        offsetLookup(map, loc, offset + vec2(-1.5, -1.5)) +
                        offsetLookup(map, loc, offset + vec2( 0.5, -1.5)) ) * 0.25;
   
   return shadowCoeff;
}
#endif

void main(void)
{
   fragColor = vec4(pcf(depthMap, stPos));
}

#else
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
#endif