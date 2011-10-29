//
// FBO example using OpenGL 3.2 / 4.1
//
// main.cpp
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//
// Changelog: 
// 2011-10-28, Initial revision <jbowles@riskybacon.com>
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


// Global variables have an underscore prefix.
GL::Program* _program;             //< GLSL program

// Array objects - quad
GLuint       _vaoQuad;             //< Array object for the vertices
GLuint       _naoQuad;             //< Array object for the normals
GLuint       _taoQuad;             //< Array object for the texture coordintes

// Array objects - cube
GLuint       _vaoCube;             //< Array object for the vertices
GLuint       _naoCube;             //< Array object for the normals
GLuint       _taoCube;             //< Array object for the texture coordintes

// Buffer objects - quad 
GLuint       _vertexBufferQuad;    //< Buffer object for the vertices
GLuint       _normalBufferQuad;    //< Buffer object for the normals
GLuint       _tcBufferQuad;        //< Buffer object for the texture coordinates

// Buffer objects - cube
GLuint       _vertexBufferCube;    //< Buffer object for the vertices
GLuint       _normalBufferCube;    //< Buffer object for the normals
GLuint       _tcBufferCube;        //< Buffer object for the texture coordinates

GLuint       _checkboard;          //< Texture object
int          _texWidth;            //< Width of the texture
int          _texHeight;           //< Height of the texture

// Shader attribute locations
GLint        _vertexLocation;      //< Location of the vertex attribute in the shader
GLint        _normalLocation;      //< Location of the normal attribute in the shader
GLint        _tcLocation;          //< Location of the texture coordinate attribute in the shader
GLint        _samplerLocation;     //< Location of the texture sampler in the fragment program

bool         _running;             //< true if the program is running, false if it is time to terminate
GLuint       _mvp;                 //< Location of the model, view, projection matrix in vertex shader
GLuint       _invTP;               //< Location of the inverse transpose of the MVP matrix
bool         _tracking;            //< True if mouse location is being tracked
Trackball*   _trackball;           //< Pointer to virtual trackball
vector<vec4> _verticesQuad;        //< Vertex data
vector<vec4> _normalsQuad;         //< Normal data
vector<vec2> _tcQuad;       //< Texture coordinate data
vector<vec4> _cubeVertices;        //< Vertex data
vector<vec4> _cubeNormals;         //< Normal data
vector<vec2> _cubeTexCoords;       //< Texture coordinate data
std::string  _vertexFile;          //< Name of the vertex shader file
std::string  _fragFile;            //< Name of the fragment shader file
GLuint       _fbo;                 //< Frame buffer object handle
GLuint       _fboTextures[2];      //< FBO related textures
GLuint       _renderbuffer;        //< Render buffer handle
int          _fboWidth;            //< Width of the FBO textures
int          _fboHeight;           //< Height of the FBO textures

enum FBOTextures
{
   DEPTH,
   RGBA
};

/**
 * Clean up and exit
 *
 * @param exitCode      The exit code, eg, EXIT_SUCCESS or EXIT_FAILURE
 */
void terminate(int exitCode)
{
   // Delete vertex buffer object
   if(_vertexBufferQuad)
   {
      glDeleteBuffers(1, &_vertexBufferQuad);
      _vertexBufferQuad = 0;
   }

   // Delete vertex array object
   if(_vaoQuad)
   {
      glDeleteVertexArrays(1, &_vaoQuad);
   }
   
   glfwTerminate();

   exit(exitCode);
}

/**
 * Check the status of an FBO
 */
void fboStatus(void)
{
   GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   std::string error;
   bool bufferComplete = false;
   switch(status)
   {
      case GL_FRAMEBUFFER_COMPLETE:
         error = "Framebuffer complete.";
         bufferComplete = true;
         break;
         
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
         error = "[ERROR] Framebuffer incomplete: Attachment is NOT complete.";
         bufferComplete = false;
         break;
         
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
         error = "[ERROR] Framebuffer incomplete: No image is attached to Framebuffer.";
         bufferComplete = false;
         break;
         
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
         error = "[ERROR] Framebuffer incomplete: Draw buffer.";
         bufferComplete = false;
         break;
         
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
         error = "[ERROR] Framebuffer incomplete: Read buffer.";
         bufferComplete = false;
         break;

      case GL_FRAMEBUFFER_UNSUPPORTED:
         error = "[ERROR] Unsupported by Framebuffer implementation.";
         bufferComplete = false;
         break;
   
      default:
         error = "[ERROR] Unknow error.";
         bufferComplete = false;
         break;
   }
         
   if(!bufferComplete)
   {
      throw GL::Exception(error);
   }
}

/**
 * Create an FBO that has an RGBA 32-bit floating point texture
 * and a texture for holding depth values
 */
void createFBO(void)
{
   
   try
   {
      _fboWidth = 256;
      _fboHeight = 256;
      glGenTextures(2, _fboTextures);
      GL_ERR_CHECK();

      for(int i = 0; i < 2; ++i)
      {
         if(_fboTextures[i] <= 0)
         {
            std::cerr << "Texture not generated" << std::endl;
         }
      }
      
      //------------------------------------------------------------------------------------------
      // Set up the texture to hold depth data
      //------------------------------------------------------------------------------------------
      glBindTexture(GL_TEXTURE_2D, _fboTextures[DEPTH]);
      GL_ERR_CHECK();
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _fboWidth, _fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      GL_ERR_CHECK();
      
      //------------------------------------------------------------------------------------------
      // Set up the RGBA texture for the rendered image
      //------------------------------------------------------------------------------------------
      glBindTexture(GL_TEXTURE_2D, _fboTextures[RGBA]);
      GL_ERR_CHECK();
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _fboWidth, _fboHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      GL_ERR_CHECK();

      //------------------------------------------------------------------------------------------
      // Set up the render buffer
      //------------------------------------------------------------------------------------------
      glGenRenderbuffers(1, &_renderbuffer);
      GL_ERR_CHECK();
      glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
      GL_ERR_CHECK();
      glRenderbufferStorage(GL_RENDERBUFFER,  GL_RGBA32F, _fboWidth, _fboHeight);
      GL_ERR_CHECK();
      

      //------------------------------------------------------------------------------------------
      // Create the frame buffer object
      //------------------------------------------------------------------------------------------
      glGenFramebuffers(1, &_fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      
      //------------------------------------------------------------------------------------------
      // Attach textures and renderbuffer to FBO
      //------------------------------------------------------------------------------------------

      // Attach RGBA / Color texture to the FBO
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fboTextures[RGBA], 0);
      GL_ERR_CHECK();
      
      // Attach depth texture to the FBO
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _fboTextures[DEPTH], 0);
      GL_ERR_CHECK();
      
      // Attach render buffer to the FBO
//      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);
      GL_ERR_CHECK();

      // Set the drawing buffer
      glDrawBuffer(GL_COLOR_ATTACHMENT0);
      GL_ERR_CHECK();
      
      // Set the reading buffer
      glReadBuffer(GL_NONE);
      GL_ERR_CHECK();

      fboStatus();

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      GL_ERR_CHECK();
      glBindTexture(GL_TEXTURE_2D, 0);
      GL_ERR_CHECK();
      // Set the drawing buffer
      glDrawBuffer(GL_BACK);
      GL_ERR_CHECK();
      // Set the reading buffer
      glReadBuffer(GL_BACK);
      GL_ERR_CHECK();
      
   } 
   catch (GL::Exception exception)
   {
      std::cerr << exception.what() << std::endl;
      terminate(EXIT_FAILURE);
   }

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
      
      std::string filename = std::string(SOURCE_DIR) + "/cat.ppm";
      
      _texWidth = 256;
      _texHeight = 256;
      
      GLfloat texels[_texWidth][_texHeight][4];

      // Create a checkerboard pattern
      for(int i = 0; i < _texWidth; i++ )
      {
         for(int j = 0; j < _texHeight; j++ )
         {
            GLubyte c = (((i & 0x8) == 0) ^ ((j & 0x8)  == 0)) * 255;
            texels[i][j][0] = c / (255.0f * 1.5f);
            texels[i][j][1] = 0;
            texels[i][j][2] = c / 255.0f;
            texels[i][j][3] = 1.0f;
         }
      }

      createFBO();
   
      glGenTextures(1, &_checkboard);
      GL_ERR_CHECK();
      glBindTexture(GL_TEXTURE_2D, _checkboard);
      GL_ERR_CHECK();
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, texels);
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      GL_ERR_CHECK();
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      GL_ERR_CHECK();
      glActiveTexture(GL_TEXTURE0);
      GL_ERR_CHECK();

      _verticesQuad.push_back(glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f));
      _verticesQuad.push_back(glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f));
      _verticesQuad.push_back(glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f));
      _verticesQuad.push_back(glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f));
      
      _normalsQuad.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalsQuad.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalsQuad.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalsQuad.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

      _tcQuad.push_back(glm::vec2(0.0f, 0.0f));
      _tcQuad.push_back(glm::vec2(1.0f, 0.0f));
      _tcQuad.push_back(glm::vec2(0.0f, 1.0f));
      _tcQuad.push_back(glm::vec2(1.0f, 1.0f));
                        
      _vertexFile = std::string(SOURCE_DIR) + "/vertex.c";
      _fragFile   = std::string(SOURCE_DIR) + "/fragment.c";
      
      _program = new GL::Program(_vertexFile, _fragFile);
      
      // Get vertex and color attribute locations
      getAttribLocations();

      // Generate a single handle for a vertex array. Only one vertex
      // array is needed
      glGenVertexArrays(1, &_vaoQuad);
      
      // Bind that vertex array
      glBindVertexArray(_vaoQuad);
      
      // Generate one handle for the vertex buffer object
      glGenBuffers(1, &_vertexBufferQuad);
      
      // Make that vbo the current array buffer. Subsequent array buffer operations
      // will affect this vbo
      //
      // It is possible to place all data into a single buffer object and use
      // offsets to tell OpenGL where the data for a vertex array or any other
      // attribute may reside.
      glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferQuad);
      GL_ERR_CHECK();
      // Set the data for the vbo. This will load it onto the GPU
      glBufferData(GL_ARRAY_BUFFER, _verticesQuad.size() * sizeof(glm::vec4),
                   &_verticesQuad[0], GL_STATIC_DRAW);
      
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
      glGenBuffers(1, &_naoQuad);
      glBindBuffer(GL_ARRAY_BUFFER, _naoQuad);
      
      glBufferData(GL_ARRAY_BUFFER, _normalsQuad.size() * sizeof(glm::vec4),
                   &_normalsQuad[0], GL_STATIC_DRAW);
      
      glVertexAttribPointer(_normalLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_normalLocation);
      
      
      // Set up texture attribute
      glGenBuffers(1, &_taoQuad);
      glBindBuffer(GL_ARRAY_BUFFER, _taoQuad);
      
      glBufferData(GL_ARRAY_BUFFER, _tcQuad.size() * sizeof(glm::vec2),
                   &_tcQuad[0], GL_STATIC_DRAW);
      
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

/**
 * Reload the shaders, but only if they compile
 */
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

/**
 * Mouse movement callback
 */ 
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
      
      //------------------------------------------------------------------------------------------
      // Draw scene into an FBO
      //------------------------------------------------------------------------------------------
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      GL_ERR_CHECK();

      // Set the viewport for the fbo
      glViewport(0, 0, _fboWidth, _fboHeight);
      GL_ERR_CHECK();

      glClearColor(0.3f, 0.4f, 0.95f, 1.0f);
      GL_ERR_CHECK();

      // Clear the color and depth buffers
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      GL_ERR_CHECK();

      glBindTexture(GL_TEXTURE_2D, _checkboard);
      GL_ERR_CHECK();

      // Draw the triangles
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _verticesQuad.size());
      GL_ERR_CHECK();

      //------------------------------------------------------------------------------------------
      // End draw FBO scene into an FBO
      //------------------------------------------------------------------------------------------

      //------------------------------------------------------------------------------------------
      // Draw the same scene into the default framebuffer
      //------------------------------------------------------------------------------------------
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      GL_ERR_CHECK();

      // Bind the checkerboard texture
      glBindTexture(GL_TEXTURE_2D, _checkboard);
      GL_ERR_CHECK();
      
      // Set the viewport for the default framebuffer
      glViewport(0, 0, width, height);

      // Clear the color and depth buffers
      glClearColor(0.3f, 0.5f, 0.9f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Draw the triangles
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _verticesQuad.size());
      GL_ERR_CHECK();
      glFlush();
      GL_ERR_CHECK();
      //------------------------------------------------------------------------------------------
      // End
      //------------------------------------------------------------------------------------------
      
      //------------------------------------------------------------------------------------------
      // Draw a textured quad that contains the RGBA texture from the FBO
      //------------------------------------------------------------------------------------------
      float scaleFactor = 0.08; //6125;
      glm::mat4 colorTrans 
         = glm::translate(glm::mat4(1.0f), glm::vec3(-0.8,0.7,0))
         * glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, 1.0f));

      glm::mat4 depthTrans 
      = glm::translate(glm::mat4(1.0f), glm::vec3(-0.6,0.7,0))
      * glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, 1.0f));

      mvp = projection * view * colorTrans;
      invTP = glm::transpose(glm::inverse(mvp));

      // Set the MVP uniform
      glUniformMatrix4fv(_mvp, 1, GL_FALSE, &mvp[0][0]);
      GL_ERR_CHECK();
      
      // Set the inverse transpose uniform
      glUniformMatrix4fv(_invTP, 1, GL_FALSE, &invTP[0][0]);
      GL_ERR_CHECK();

      glBindTexture(GL_TEXTURE_2D, _fboTextures[RGBA]);
      GL_ERR_CHECK();
      
      // Draw the triangles
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _verticesQuad.size());
      GL_ERR_CHECK();

      mvp = projection * view * depthTrans;
      invTP = glm::transpose(glm::inverse(mvp));
      
      // Set the MVP uniform
      glUniformMatrix4fv(_mvp, 1, GL_FALSE, &mvp[0][0]);
      GL_ERR_CHECK();
      
      // Set the inverse transpose uniform
      glUniformMatrix4fv(_invTP, 1, GL_FALSE, &invTP[0][0]);
      GL_ERR_CHECK();
      
      glBindTexture(GL_TEXTURE_2D, _fboTextures[DEPTH]);
      GL_ERR_CHECK();
      
      // Draw the triangles
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _verticesQuad.size());
      GL_ERR_CHECK();
      //------------------------------------------------------------------------------------------
      // End
      //------------------------------------------------------------------------------------------

      glBindTexture(GL_TEXTURE_2D, 0);
      GL_ERR_CHECK();
   } 
   catch (GL::Exception exception)
   {
      std::cerr << exception.what() << std::endl;
      terminate(EXIT_FAILURE);
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
