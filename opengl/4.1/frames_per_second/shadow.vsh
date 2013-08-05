#version 150
// Shadow mapping with diffuse lighting

// Input attributes: position, normal, texture coordinate
in vec4 vertex;
in vec4 normal;
in vec2 tc;

// Transformation matrices
uniform mat4 mvp;
uniform mat4 toShadowTex;

out vec4 stPos;        //< Shadow texture position

void main(void)
{
   // Transform vertex into canonical view volume
   gl_Position = mvp  * vertex;

   // Transform vertex position to shadow map position
   // Transformations applied:
   // model -> world -> light view -> light projection -> [-1,1] -> [0,1]
   stPos  = toShadowTex * vertex;
}
