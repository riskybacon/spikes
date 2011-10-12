#version 150

in vec4 vertex;
in vec4 normal;
in vec2 tc;

out vec4 inFragColor;
uniform mat4 mvp;
uniform mat4 invTP;

void main(void)
{
   gl_Position = mvp * vertex;
   vec4 lightPos = vec4(40,0,0, 1.0f);
   vec3 lightDir =  lightPos.xyz - gl_Position.xyz;
   lightDir = normalize(lightDir);
   vec4 rotNormal = invTP * normal;
   
   vec3 dp = dot(lightDir, rotNormal.xyz) * vec3(0.9, 0.6, 0.5);
   
   inFragColor = vec4(dp, 1.0);
}
