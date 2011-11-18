// Pass-thru vertex shader. Transformations are performed
// in the geometry shader in this example.
#version 150

in vec4 vertex;
in vec4 color;
out vec4 geomColor;

// Modelview matrix
uniform mat4 mv;

void main(void)
{
   gl_Position = mv * vertex;
   geomColor = color;
}
