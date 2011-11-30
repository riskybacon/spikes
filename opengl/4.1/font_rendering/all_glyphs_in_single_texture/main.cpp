//
// OBJ model loader using OpenGL 3.2 / 4.1
//
// main.cpp
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//
// Changelog: 
// 2011-10-03, Initial revision <jbowles@riskybacon.com>
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



using namespace glm;
using std::vector;

#define _DEBUG

#include "platform_specific.h" // Include this file before glfw.h
#include <GL/glfw.h>
#include "trackball.h"
#include "oglwrapper.h"
#include "Font.h"


// Global variables have an underscore prefix.
GL::Program* _program;         //< GLSL program
GLuint       _vao;             //< Array object for the vertices
GLuint       _nao;             //< Array object for the normals
GLuint       _tao;             //< Array object for the texture coordintes
GLuint       _vertexBuffer;    //< Buffer object for the vertices
GLuint       _normalBuffer;    //< Buffer object for the normals
GLuint       _tcBuffer;        //< Buffer object for the texture coordinates
GLuint       _texture;         //< Texture object
int          _texWidth;        //< Width of the texture
int          _texHeight;       //< Height of the texture
GLint        _vertexLocation;  //< Location of the vertex attribute in the shader
GLint        _normalLocation;  //< Location of the normal attribute in the shader
GLint        _tcLocation;      //< Location of the texture coordinate attribute in the shader
GLint        _samplerLocation; //< Location of the texture sampler in the fragment program
bool         _running;         //< true if the program is running, false if it is time to terminate
GLuint       _mvp;             //< Location of the model, view, projection matrix in vertex shader
GLuint       _invTP;           //< Location of the inverse transpose of the MVP matrix
bool         _tracking;        //< True if mouse location is being tracked
Trackball*   _trackball;       //< Pointer to virtual trackball
vector<vec4> _vertexData;      //< Vertex data
vector<vec4> _normalData;      //< Normal data
vector<vec2> _tcData;          //< Texture coordinate data
std::string  _vertexFile;      //< Name of the vertex shader file
std::string  _fragFile;        //< Name of the fragment shader file
Font*        _font;
GLuint       _fontTexID;       //< Texture ID for the font

/**
 * Clean up and exit
 *
 * @param exitCode      The exit code, eg, EXIT_SUCCESS or EXIT_FAILURE
 */
void terminate(int exitCode)
{
   // Delete vertex buffer object
   if(_vertexBuffer)
   {
      glDeleteBuffers(1, &_vertexBuffer);
      _vertexBuffer = 0;
   }

   // Delete vertex array object
   if(_vao)
   {
      glDeleteVertexArrays(1, &_vao);
   }
   
   glfwTerminate();

   exit(exitCode);
}

/**
 *  Initialize OpenGL Extension Wrangler. 
 *  Does nothing under OS X
 */
void initGLEW(void)
{
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
}

/**
 * Get locations of attributes
 */

void getAttribLocations(void)
{
   try 
   {
      _vertexLocation  = _program->getAttribLocation("vertex");
      _normalLocation  = _program->getAttribLocation("normal");
      _tcLocation      = _program->getAttribLocation("tc");
      _mvp             = _program->getUniformLocation("mvp");
      _invTP           = _program->getUniformLocation("invTP");
      _samplerLocation = _program->getUniformLocation("tex");
      GL_ERR_CHECK();
   }
   catch(GL::Exception exception)
   {
      std::cerr << exception.what() << std::endl;
   }
}
/**
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
void init(void)
{
   try
   {
      initGLEW();
      
     std::string fontFile = std::string(SOURCE_DIR) + "/HelveticaLight.ttf";
     _font = new Font(fontFile, 32);
     
     // Turn the font into a texture
     glGenTextures(1, &_fontTexID);
     glBindTexture(GL_TEXTURE_2D, _fontTexID);
     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     GL_ERR_CHECK();
     glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
     GL_ERR_CHECK();
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
     GL_ERR_CHECK();
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
     GL_ERR_CHECK();
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _font->texWidth(), _font->texHeight(), 0, GL_RED, GL_FLOAT, _font->data());
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _font->texWidth(), _font->texHeight(), 0, GL_RGBA, GL_FLOAT, _font->data());
     GL_ERR_CHECK();

      glActiveTexture(GL_TEXTURE0);
      GL_ERR_CHECK();
      
     unsigned char glyph = 'q';
     float width = _font->glyphWidth(glyph) * 0.5;
     float height = _font->glyphHeight(glyph) * 0.5;
     width = 1.0f;
     height = 1.0f;
      _vertexData.push_back(glm::vec4(-1.0f * width, -1.0f * height, 0.0f, 1.0f));
      _vertexData.push_back(glm::vec4( 1.0f * width, -1.0f * height, 0.0f, 1.0f));
      _vertexData.push_back(glm::vec4(-1.0f * width,  1.0f * height, 0.0f, 1.0f));
      _vertexData.push_back(glm::vec4( 1.0f * width,  1.0f * height, 0.0f, 1.0f));
      
      _normalData.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalData.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalData.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalData.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

     float xMin, xMax, yMin, yMax;
     _font->texCoords(glyph, xMin, xMax, yMin, yMax);
     std::cout << "(xMin, yMin) , (xMax, yMax): (" << xMin << "," << yMin << "),(" << xMax << "," << yMax << ")" << std::endl;
#if 0
     _tcData.push_back(glm::vec2(xMin, yMin));
     _tcData.push_back(glm::vec2(xMax, yMin));
     _tcData.push_back(glm::vec2(xMin, yMax));
     _tcData.push_back(glm::vec2(xMax, yMax));
#else
     _tcData.push_back(glm::vec2(0,0));
     _tcData.push_back(glm::vec2(1,0));
     _tcData.push_back(glm::vec2(0,1));
     _tcData.push_back(glm::vec2(1,1));
#endif
      _vertexFile = std::string(SOURCE_DIR) + "/vertex.c";
      _fragFile   = std::string(SOURCE_DIR) + "/fragment.c";
      
      _program = new GL::Program(_vertexFile, _fragFile);
      
      // Get vertex and color attribute locations
      getAttribLocations();

      // Generate a single handle for a vertex array. Only one vertex
      // array is needed
      glGenVertexArrays(1, &_vao);
      
      // Bind that vertex array
      glBindVertexArray(_vao);
      
      // Generate one handle for the vertex buffer object
      glGenBuffers(1, &_vertexBuffer);
      
      // Make that vbo the current array buffer. Subsequent array buffer operations
      // will affect this vbo
      //
      // It is possible to place all data into a single buffer object and use
      // offsets to tell OpenGL where the data for a vertex array or any other
      // attribute may reside.
      glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
      GL_ERR_CHECK();
      
      // Set the data for the vbo. This will load it onto the GPU
      glBufferData(GL_ARRAY_BUFFER, _vertexData.size() * sizeof(glm::vec4),
                   &_vertexData[0], GL_STATIC_DRAW);
      
      // Specify the location and data format of the array of generic vertex attributes
      glVertexAttribPointer(_vertexLocation, // Attribute location in the shader program
                            4,               // Number of components per attribute
                            GL_FLOAT,        // Data type of attribute
                            GL_FALSE,        // GL_TRUE: values are normalized or
                            0,               // Stride
                            0);              // Offset into currently bound array buffer for this data
      
      // Enable the generic vertex attribute array
      glEnableVertexAttribArray(_vertexLocation);
      
      // Set up normal attribute
      glGenBuffers(1, &_nao);
      glBindBuffer(GL_ARRAY_BUFFER, _nao);
      
      glBufferData(GL_ARRAY_BUFFER, _normalData.size() * sizeof(glm::vec4),
                   &_normalData[0], GL_STATIC_DRAW);
      
      glVertexAttribPointer(_normalLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_normalLocation);
      
      
      // Set up texture attribute
      glGenBuffers(1, &_tao);
      glBindBuffer(GL_ARRAY_BUFFER, _tao);
      
      glBufferData(GL_ARRAY_BUFFER, _tcData.size() * sizeof(glm::vec2),
                   &_tcData[0], GL_STATIC_DRAW);
      
      glVertexAttribPointer(_tcLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_tcLocation);
      
      // Set the clear color
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      
      // Set the depth clearing value
      glClearDepth(1.0f);
      
      // Enable depth test
      glEnable(GL_DEPTH_TEST);
      GL_ERR_CHECK();


   } 
   catch (GL::Exception exception)
   {
      std::cerr << exception.what() << std::endl;
      terminate(EXIT_FAILURE);
   }
  
}

void reloadShaders(void)
{
   try
   {
      GL::Program* newProgram = new GL::Program(_vertexFile, _fragFile);
      delete _program;
      _program = newProgram;
      
   }
   catch (GL::Exception exception)
   {
      std::cerr << exception.what() << std::endl;
   }
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
   
   _trackball->reshape(width, height);
}

/**
 *  Mouse click callback
 *
 *  @param button that was clicked
 *  @param button state
 */
void GLFWCALL mouseButton(int button, int action)
{
   if(button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
   {
      _tracking = !_tracking;
      
   }
   
   if(_tracking)
   {
      int x, y;
      glfwGetMousePos(&x, &y);
      _trackball->start(x,y);
   }
   else
   {
      _trackball->stop();
   }
}

void GLFWCALL mouseMove(int x, int y)
{
   if(_tracking)
   {
      int width, height;
      glfwGetWindowSize(&width, &height);
      _trackball->motion(x, height - y);
   }
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
   try
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
                                              4000.0f);                      // Far clip
      // Camera matrix
      glm::mat4 view       = glm::lookAt(glm::vec3(0,0,2), // Camera position is at (4,3,3), in world space
                                         glm::vec3(0,0,0), // and looks at the origin
                                         glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                         );
      
      glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
      
    
      // Get vertex and color attribute locations
      getAttribLocations();

      // Model matrix 
      glm::mat4 model = _trackball->getTransform();
      
      // Create  model, view, projection matrix
      glm::mat4 mvp        = projection * view * translate * model; // Remember, matrix multiplication is the other way around
      
      // Calculate the inverse transpose for use with normals
      glm::mat4 invTP      = transpose(glm::inverse(mvp));
      
      // Use the shader program that was loaded, compiled and linked
      _program->bind();
      GL_ERR_CHECK();
      
      // Set the MVP uniform
      glUniformMatrix4fv(_mvp, 1, GL_FALSE, &mvp[0][0]);
      GL_ERR_CHECK();
      
      // Set the inverse transpose uniform
      glUniformMatrix4fv(_invTP, 1, GL_FALSE, &invTP[0][0]);
      GL_ERR_CHECK();
      
      // Draw the triangles
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _vertexData.size());
      GL_ERR_CHECK();
      
   } 
   catch (GL::Exception exception)
   {
      std::cerr << exception.what() << std::endl;
   }

   glfwSwapBuffers();
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
   _tracking = false;
   _trackball = new Trackball(width, height);
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
   glfwSetMouseButtonCallback(mouseButton);
   glfwSetMousePosCallback(mouseMove);
   
   std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;

   init();
   resize(width, height);
  
   // Main loop. Run until ESC key is pressed or the window is closed
   while(_running)
   {
      update(glfwGetTime());
   }

   terminate(EXIT_SUCCESS);
}
