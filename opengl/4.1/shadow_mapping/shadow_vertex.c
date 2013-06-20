#version 150

in vec4 vertex;
in vec4 normal;
in vec2 tc;

uniform mat4 mvp;
uniform mat4 invTP;
uniform mat4 toShadowTex;

out vec3 N;
out vec3 v;

out vec4 stPos; //< Shadow texture position
out vec4 cmPos; //< Camera space position

out vec2 fragTC;

void main(void)
{
   // Transform vertex into canonical view volume
   gl_Position = mvp      * vertex;

   stPos       = toShadowTex * vertex;
   stPos /= stPos.w;
   
   cmPos        = gl_Position;
   
   N = (normalize(invTP * normal)).xyz;
   v = gl_Position.xyz;
   
   fragTC = tc;
}
