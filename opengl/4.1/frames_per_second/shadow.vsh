#version 150
// Shadow mapping with diffuse lighting

// Input attributes: position, normal, texture coordinate
in vec4 vertex;
in vec4 normal;
in vec2 tc;

// Transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 mvp;
uniform mat4 toShadowTex;

// Position of light in world space
uniform vec4 worldLightPos;

out vec3 N;            //< Normal transformed
out vec4 stPos;        //< Shadow texture position
out vec2 fragTC;       //< Texture coordinate
out vec4 worldPos;     //< Position of fragment in world space
out vec4 viewLightPos; //< Position of light in view space

void main(void)
{
   // Transform vertex into canonical view volume
   gl_Position = mvp  * vertex;

   // Get the position of this vertex in world space. This
   // will be used for lighting
   worldPos = model * vertex;
   
   // Get the position of the light in view space
   viewLightPos = view * worldLightPos;
   
   // Transform vertex position to shadow map position
   // Transformations applied:
   // model -> world -> light view -> light projection -> [-1,1] -> [0,1]
   stPos  = toShadowTex * vertex;
  
   // Transform the normal using the inverse transpose. Using
   // the mvp doesn't work in some cases, particulary when
   // scaling and shearing occur
   mat4 itp = transpose(inverse(model));
   N = (normalize(itp * normal)).xyz;
   
   // Texture coordinate goes through unchanged
   fragTC = tc;
}
