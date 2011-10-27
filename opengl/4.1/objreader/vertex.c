#version 150

in vec4 vertex;
in vec4 normal;
in vec2 tc;

out vec4 inFragColor;
uniform mat4 mvp;
uniform mat4 invTP;

void main(void)
{
   // Transform vertex into view volume
   gl_Position = mvp * vertex;
 
   // Set the light position, this should be a uniform variable
   // and passed in.
   vec4 lightPos = vec4(40,10,0, 1.0f);
   
   // Get the direction from the vertex to the light
   vec3 lightDir =  lightPos.xyz - gl_Position.xyz;
   lightDir = normalize(lightDir);
   
   // Use inverse transpose of model/view/projection matrix
   // to transform normals
   vec4 rotNormal = invTP * normal;
   
   // Get the diffuse lighting for the model
   vec3 dp = dot(lightDir, rotNormal.xyz) * vec3(0.9, 0.6, 0.5);
   
   // Output the fragment color.
   inFragColor = vec4(dp, 1.0);
}
