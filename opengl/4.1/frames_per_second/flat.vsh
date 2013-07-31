#version 150
// Flat shading vertex shader. No lighting, just transform the
// vertices into the canonical view volume and pass to the 
// rest of the pipeline

// Input vertices
in vec4 vertex;

// Model, view, projection matrix
uniform mat4 mvp;

void main(void)
{
   // Transform vertex into view volume
   gl_Position = mvp * vertex;
}
