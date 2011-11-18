// Pass-thru geometry shader for triangles
#version 150

// Takes in 3 points for a triang;e
layout(triangles) in;

// Outputs 3 points for a triangle strip, which
// ends up being a single triangle
layout(triangle_strip, max_vertices = 3) out;

// Modelview matrix
uniform mat4 mv;

// Projection matrix
uniform mat4 proj;

// Input color for the incoming vertices. This
// matches up to inGeomColor in the vertex shader
in vec4 geomColor[3];

// The color for the fragment shader
out vec4 vertexColor;

void main() {
   // Iterate over each incoming vertex
   for(int i = 0; i < 3; i++)
   {
      // Transform the vertex into the view plane
      gl_Position = proj * mv * gl_in[i].gl_Position;
      // Set the out color for this vertex
      vertexColor = geomColor[i];
      // Emit the vertex
      EmitVertex();
   }
   // Done composing the primitive
   EndPrimitive();
}