#version 150

in vec3 N;
in vec3 v;
in vec2 fragTC;
in vec4 stPos;
in vec4 cmPos;
uniform vec4 lightPos;
uniform sampler2D depthMap;

out vec4 fragColor;


void main(void)
{
   // Default light attenuation factor
   float attenuation = 1.0;
   float occludingDepth = texture(depthMap, stPos.xy).r;
   
   // Shadow mapping happens here - attenuate the light if the distance
   // of the occluding object to the light is less than the distance of
   // the receiver to the light.
   attenuation = occludingDepth < stPos.z - 0.00001 ? 0.75 : attenuation;
   
   vec4 diffuseMaterial = vec4(0.9, 0.6, 0.5, 1.0);
   vec4 specularMaterial = vec4(1, 0, 0, 1);
   float shininess = 100;
   
   vec3 E = normalize(-v);
   vec3 L = normalize(lightPos.xyz - v);
   vec3 R = reflect(-L, N);
   
   float specDP = max(dot(R,E), 0);
   vec4 specular = specularMaterial * pow(specDP, shininess);
   vec4 diffuse = diffuseMaterial * max(dot(N,L), 0);
   
   fragColor = clamp(attenuation * (diffuse + specular), 0, 1);
}
