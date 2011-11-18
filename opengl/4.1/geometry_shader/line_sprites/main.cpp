//
// Rotating triangle demo using OpenGL 3.2 / 4.1
//
// main.cpp
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//
// Changelog: 
// 2011-10-03, Initial revision <jbowles@riskybacon.com>
//

#define _DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using std::vector;
using std::string;

// Include this file before glfw.h
#include "platform_specific.h"
 
#include <GL/glfw.h>

#include "Shader.h"

// Global variables have an underscore prefix.
GL::Program* _program;  //< Shader program handle
GLuint _vao;            //< Vertex array object for the vertices
GLuint _cao;            //< Vertex array object for the colors
GLuint _iao;            //< Array object for the indices
GLuint _vertices;       //< Vertex buffer object for the vertices
GLuint _colors;         //< Vertex buffer object for the colors
GLuint _indices;        //< Buffer object for the vertex indices
GLint  _vertexLocation; //< Location of the vertex attribute in the shader program
GLint  _colorLocation;  //< Location of the color attribute in the shader program
bool   _running;        //< true if the program is running, false if it is time to terminate
GLuint _mvp;            //< Location of the model, view, projection matrix in vertex shader
string _vertexFile;     //< Path to the vertex shader;
string _fragFile;       //< Path to the fragment shader
string _geomFile;       //< Path to the geometry shader
/**
 * Clean up and exit
 *
 * @param exitCode      The exit code, eg, EXIT_SUCCESS or EXIT_FAILURE
 */
void terminate(int exitCode)
{
   // Delete vertex buffer object
   if(_vertices)
   {
      glDeleteBuffers(1, &_vertices);
      _vertices = 0;
   }

   // Delete vertex array object
   if(_vao)
   {
      glDeleteVertexArrays(1, &_vao);
   }
   
   // Delete shader program
   if(_program)
   {
      delete _program;
   }

   glfwTerminate();

   exit(exitCode);
}

void reloadShaders(void)
{
   try 
   {
      GL::Program* temp = new GL::Program(_vertexFile, _fragFile, _geomFile);
      delete _program;
      _program = temp;
   }
   catch(std::runtime_error e)
   {
      std::cerr << e.what() << std::endl;
   }

}
/**
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
void init(void)
{
   // Vertices of a unit cube centered at origin, sides aligned with axes
   vector<vec4> points;
   points.push_back(vec4( -0.5, -0.5,  0.5, 1.0 ));
   points.push_back(vec4( -0.5,  0.5,  0.5, 1.0 ));
   points.push_back(vec4(  0.5,  0.5,  0.5, 1.0 ));
   points.push_back(vec4(  0.5, -0.5,  0.5, 1.0 ));
   points.push_back(vec4( -0.5, -0.5, -0.5, 1.0 ));
   points.push_back(vec4( -0.5,  0.5, -0.5, 1.0 ));
   points.push_back(vec4(  0.5,  0.5, -0.5, 1.0 ));
   points.push_back(vec4(  0.5, -0.5, -0.5, 1.0 ));
   
   // RGBA olors
   vector<vec4> colors;
   colors.push_back(vec4( 0.0, 0.0, 0.0, 1.0 ));  // black
   colors.push_back(vec4( 1.0, 0.0, 0.0, 1.0 ));  // red
   colors.push_back(vec4( 1.0, 1.0, 0.0, 1.0 ));  // yellow
   colors.push_back(vec4( 0.0, 1.0, 0.0, 1.0 ));  // green
   colors.push_back(vec4( 0.0, 0.0, 1.0, 1.0 ));  // blue
   colors.push_back(vec4( 1.0, 0.0, 1.0, 1.0 ));  // magenta
   colors.push_back(vec4( 1.0, 1.0, 1.0, 1.0 ));  // white
   colors.push_back(vec4( 0.0, 1.0, 1.0, 1.0 ));  // cyan

   // Vertex index order
   vector<GLuint> indices;
   indices.push_back(1); // Face 1, two triangles
   indices.push_back(0);
   indices.push_back(3);
   indices.push_back(1);
   indices.push_back(3);
   indices.push_back(2); 

   indices.push_back(2);
   indices.push_back(3);
   indices.push_back(7);
   indices.push_back(2);
   indices.push_back(7);
   indices.push_back(6);
   
   indices.push_back(3);
   indices.push_back(0);
   indices.push_back(4);
   indices.push_back(3);
   indices.push_back(4);
   indices.push_back(7);
   
   indices.push_back(6);
   indices.push_back(5);
   indices.push_back(1);
   indices.push_back(6);
   indices.push_back(1);
   indices.push_back(2);
   
   indices.push_back(4);
   indices.push_back(5);
   indices.push_back(6);
   indices.push_back(4);
   indices.push_back(6);
   indices.push_back(7);
   
   indices.push_back(5); // Face 6, two triangles
   indices.push_back(4);
   indices.push_back(0);
   indices.push_back(5);
   indices.push_back(0);
   indices.push_back(1); 
   
#ifndef __APPLE__
   // GLEW has trouble supporting the core profile
   glewExperimental = GL_TRUE;
   glewInit();
   if(!GLEW_ARB_vertex_array_object)
   {
      std::cerr << "ARB_vertex_array_object not available." << std::endl; 
      terminate(EXIT_FAILURE);
   }
#endif
   
   _vertexFile = std::string(SOURCE_DIR) + "/vertex.c";
   _fragFile   = std::string(SOURCE_DIR) + "/fragment.c";
   _geomFile   = std::string(SOURCE_DIR) + "/geometry.c";
   _program = NULL;
   reloadShaders();
   if(_program == NULL) {
      std::cerr << "Couldn't load shaders" << std::endl;
      terminate(EXIT_FAILURE);
   }

   _program->bind();
   
   // Get vertex and color attribute locations
   _vertexLocation = _program->getAttribLocation("vertex");
   _colorLocation  = _program->getAttribLocation("color");
      
   // Generate a single handle for a vertex array. Only one vertex
   // array is needed
   glGenVertexArrays(1, &_vao);
   
   // Bind that vertex array
   glBindVertexArray(_vao);
   
   // Generate one handle for the vertex buffer object
   glGenBuffers(1, &_vertices);
   
   // Make that vbo the current array buffer. Subsequent array buffer operations
   // will affect this vbo
   //
   // It is possible to place all data into a single buffer object and use
   // offsets to tell OpenGL where the data for a vertex array or any other
   // attribute may reside.
   glBindBuffer(GL_ARRAY_BUFFER, _vertices);
   
   // Set the data for the vbo. This will load it onto the GPU
   glBufferData(GL_ARRAY_BUFFER,                   // Target buffer object
                points.size() * sizeof(glm::vec4), // Size in bytes of the buffer 
                &points[0],                        // Pointer to the data
                GL_STATIC_DRAW);                   // Expected data usage pattern
   
   // Specify the location and data format of the array of generic vertex attributes
   glVertexAttribPointer(_vertexLocation, // Attribute location in the shader program
                         4,               // Number of components per attribute
                         GL_FLOAT,        // Data type of attribute
                         GL_FALSE,        // GL_TRUE: values are normalized to (-1, 1)
                         0,               // Stride
                         0);              // Offset into VBO for this data
   
   // Enable the generic vertex attribute array
   glEnableVertexAttribArray(_vertexLocation);
   
   // Set up color attributes
   glGenBuffers(1, &_colors);
   glBindBuffer(GL_ARRAY_BUFFER, _colors);
   glBufferData(GL_ARRAY_BUFFER,
                colors.size() * sizeof(glm::vec4),
                &colors[0],
                GL_STATIC_DRAW);
   _colorLocation = _program->getAttribLocation("color");
   glVertexAttribPointer(_colorLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(_colorLocation);

   // Create the index buffer
   glGenBuffers(1, &_indices);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

   // Set the clear color
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   
   // Set the depth clearing value
   glClearDepth(1.0f);
   
   // Enable depth test
   glEnable(GL_DEPTH_TEST);
}

/**
 * Window resize callback
 * 
 * @param width   the width of the window
 * @param height  the height of the window
 */
void GLFWCALL resize(int width, int height)
{
   // Set the affine transform of (x,y) from normalized device coordinates to
   // window coordinates. In this case, (-1,1) -> (0, width) and (-1,1) -> (0, height)
   glViewport(0, 0, width, height);
}

/**
 * Keypress callback
 */ 
void GLFWCALL keypress(int key, int state)
{
   if(state == GLFW_PRESS)
   {
      switch(key)
      {
         case GLFW_KEY_ESC:
            _running = false;
            break;
         case 'R':
         case 'r':
            reloadShaders();
            break;
      }
   }
}

/**
 * Window close callback
 */
int GLFWCALL close(void)
{
   _running = false;
   return GL_TRUE;
}

/**
 * Main loop
 * @param time    time elapsed in seconds since the start of the program
 */
int update(double time)
{
   // Clear the color and depth buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Get the width and height of the window
   int width;
   int height;
   glfwGetWindowSize(&width, &height);
   
   // Projection matrix
   glm::mat4 projection = glm::perspective(45.0f,                         // 45 degree field of view
                                           float(width) / float(height),  // Ratio
                                           0.1f,                          // Near clip 
                                           100.0f);                       // Far clip
   // Camera matrix
   glm::mat4 view       = glm::lookAt(glm::vec3(2,3,4), // Camera position is at (4,3,3), in world space
                                      glm::vec3(0,0,0), // and looks at the origin
                                      glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                      );

   // Rotation axis
   glm::vec3 axis(0.0, 1.0, 0.0);
   
   // Rotation matrix - rotate once per second
   
   float angle = float(time) * 90;
   
   glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
   
   // Model matrix 
   glm::mat4 model      = rotation;
   
   // Create  model, view, projection matrix
   glm::mat4 mvp        = projection * view * model; // Remember, matrix multiplication is the other way around
   glm::mat4 mv = view * model;
   
   // Use the shader program that was loaded, compiled and linked
   _program->bind();
   
   // Set the MVP uniform
//   _program->setUniformMatrix4fv("mvp",  1, GL_FALSE, &mvp[0][0]);
//   glUniformMatrix4fv(1, 1, GL_FALSE, &mv[0][0]);
   GL_ERR_CHECK();

   _program->setUniformMatrix4fv("proj",  1, GL_FALSE, &projection[0][0]);
   _program->setUniformMatrix4fv("mv",    1, GL_FALSE, &mv[0][0]);
   
   glDrawElements(GL_LINES,         // Primitive to draw
                  6 * 6,            // Number of indices
                  GL_UNSIGNED_INT,  // Data type of index values
                  NULL);            // Pointer to the data (NULL if data onGPUr already)

   return GL_TRUE;
}

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
   int width = 1024; // Initial window width
   int height = 768; // Initial window height
   _running = true;

   // Initialize GLFW
   glfwInit();

   // Request an OpenGL core profile context, without backwards compatibility
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR,  GL_MAJOR);
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR,  GL_MINOR);
   glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwOpenWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
   glfwOpenWindowHint(GLFW_FSAA_SAMPLES,          4 );

   // Open a window and create its OpenGL context
   if(!glfwOpenWindow(width, height, 0,0,0,0, 32,0, GLFW_WINDOW ))
   {
      std::cerr << "Failed to open GLFW window" << std::endl;
      glfwTerminate();
      return -1;
   }

   glfwSetWindowSizeCallback(resize);
   glfwSetKeyCallback(keypress);
   glfwSetWindowCloseCallback(close);

   std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;

   init();
   resize(width, height);
  
   // Main loop. Run until ESC key is pressed or the window is closed
   while(_running)
   {
      update(glfwGetTime());
      glfwSwapBuffers();
   }

   terminate(EXIT_SUCCESS);
}
