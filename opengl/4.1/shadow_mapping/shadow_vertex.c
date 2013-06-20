#version 150

in vec4 vertex;
in vec4 normal;
in vec2 tc;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 mvp;
uniform mat4 invTP;
uniform vec3 lightPos;

out vec4 vertexColor;

void main(void)
{
   // Transform vertex into world space
   vec4 worldPos = model * vertex;

   // Transform vertex into canonical view volume
   gl_Position = mvp * vertex;

   // Get a vector pointing from the 
   vec3 lightDir = normalize(lightPos - worldPos.xyz);

   // Use inverse transpose of mvp to transform normals
   vec4 transNormal = invTP * normal;

   // Get the diffuse lighting component
   vec3 dp = dot(lightDir, transNormal.xyz) * vec3(0.9, 0.6, 0.5);

   // Output the color for this vertex
   vertexColor = vec4(dp, 1.0);
}
