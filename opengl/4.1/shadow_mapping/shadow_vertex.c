#version 150
// Shadow mapping with very simple Phong shading.

// Input attributes: position, normal, texture coordinate
in vec4 vertex;
in vec4 normal;
in vec2 tc;

// Transformation matrices
uniform mat4 model;
uniform mat4 mvp;
uniform mat4 invTP;
uniform mat4 toShadowTex;

out vec3 N;      //< Normal transformed

out vec4 stPos;  //< Shadow texture position

out vec2 fragTC; //< Texture coordinate;

out vec4 vertexNormal;
out vec4 worldPos;

void main(void)
{
   // Transform vertex into canonical view volume
   gl_Position = mvp      * vertex;

   mat4 itp = transpose(inverse(model));
   
   // Get the position of this vertex in world space. This
   // will be used for lighting
   worldPos = model * vertex;
   
   // Transform vertex position to shadow map position
   // Transformations applied
   // model -> world -> light view -> light projection -> [-1,1] -> [0,1]
   stPos  = toShadowTex * vertex;
   //   stPos /= stPos.w;
  
   // Phong shading - mostly performed in the fragment shader.

   // Transform the normal into light space using the inverse
   // transpose of the mvp matrix
   N = (normalize(itp * normal)).xyz;
   //   v = gl_Position.xyz;
   
   // Texture coordinate goes through unchanged
   fragTC = tc;
   
   vertexNormal = normal;
}
