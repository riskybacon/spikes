// Pass-thru geometry shader for triangles
#version 150

// Takes in 3 points for a triang;e
layout(lines) in;

// Outputs 3 points for a triangle strip, which
// ends up being a single triangle
layout(line_strip, max_vertices = 2) out;

// Projection matrix
uniform mat4 proj;

// Input color for the incoming vertices. This
// matches up to inGeomColor in the vertex shader
in vec4 geomColor[2];

// The color for the fragment shader
out vec4 vertexColor;

void main() {

   // Pick one of the endpoints, and translate
   // it to the origin in the x,y plane.
   mat4 trans = 
      mat4 (1, 0, 0, -gl_in[0].gl_Position.x,
            0, 1, 0, -gl_in[0].gl_Position.y,
            0, 0, 1, 0,
            0, 0, 0, 1);
   
   // Inverse translation
   mat4 transInv =
      mat4 (1, 0, 0, gl_in[0].gl_Position.x,
            0, 1, 0, gl_in[0].gl_Position.y,
            0, 0, 1, 0,
            0, 0, 0, 1);
   
   vec4 pos[6];
   pos[2] = gl_in[0].gl_Position; // At (x,y) origin
   pos[3] = gl_in[1].gl_Position; // Not at (x,y) origin
   
   
   float width = 0.5;
   
   mat4 moveUp = 
      mat4(1, 0, 0, 0,
           0, 1, 0, width,
           0, 0, 1, 0,
           0, 0, 0, 1);

   mat4 moveDown = 
      mat4(1, 0, 0, 0,
           0, 1, 0, -width,
           0, 0, 1, 0,
           0, 0, 0, 1);

#if 0
   // Rotate about the z-axis to get the line
   // into the x / z plane. Actually, we know
   // what the end result of that rotation will
   // look like without performing the rotation,
   // all that is need to be known is the length
   // of the line. 
   //
   // What is needed is the inverse of the 
   // rotation matrix to rotate the new points
   // back into their original basis
   float cosTheta;
   float sinTheta;
   float d = sqrt(pos[3].x * pos[3].x + pos[3].y * pos[3].y);

   pos[3].y = 0;

   if(pos[1].x >= 0 && pos[1].y >= 0) {
      sinTheta = pos[1].y / d;
      cosTheta = pos[1].x / d;
      pos[3].x = d;
   }

   if(pos[1].x < 0 && pos[1].y >= 0) {
      // Make sin(theta) -sin(theta) for
      // the inv rotation. -cos(theta) = cos(theta)
      sinTheta = -pos[1].y / d;
      cosTheta = -pos[1].x / d;
      pos[3].x = -d;
   }

   if(pos[1].x < 0 && pos[1].y < 0) {
      sinTheta = -pos[1].y / d;
      cosTheta = -pos[1].x / d;
      pos[3].x = -d;
      pos[3].y = 0;
   }

   if(pos[1].x >= 0 && pos[1].y < 0) {
      // Make sin(theta) -sin(theta) for
      // the inv rotation. -cos(theta) = cos(theta)
      sinTheta = pos[1].y / d;
      cosTheta = pos[1].x / d;
      pos[3].x = d;
   }

   mat4 rotInv =
      mat4 (cosTheta, sinTheta, 0, 0,
            sinTheta, cosTheta, 0, 0,
            0,        0,        1, 0,
            0,        0,        0, 1);
   
   vec4 width = vec4(0, 0.5, 0, 0) * 0.1;
   
   pos[0] = pos[2] + width;
   pos[4] = pos[2] - width;
   pos[1] = pos[3] + width;
   pos[5] = pos[3] - width;
   
   // Emit transformed vertices
   for(int i = 0; i < 6; i++)
   {
      // Transform the vertex into the view plane
      gl_Position = proj * transInv * pos[i];
      // Set the out color for this vertex
      vertexColor = geomColor[i];
      // Emit the vertex
      EmitVertex();
   }
#endif
   
   gl_Position = proj * transInv * pos[2];
   vertexColor = geomColor[0];
   EmitVertex();
   gl_Position = proj * transInv * pos[3];
   vertexColor = geomColor[1];
   EmitVertex();
   
   // Done composing the primitive
   EndPrimitive();
}