//
// FBO example using OpenGL 3.2
//
// main.cpp
//
// Author: Jeff Bowles <jbowles@riskybacon.com>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // rotation, translation, scale, etc
#include <glm/gtc/type_ptr.hpp>         // value_ptr

using namespace glm;
using std::vector;

// Comment this out to turn GL_ERR_CHECK into a no-op
//#define _DEBUG

#ifdef __APPLE__
#  define GLFW_INCLUDE_GLCOREARB
#else
// Non-Apple platforms need the GL Extension Wrangler
#  include <GL/glew.h>
#  if !defined(_WIN32) && !defined(_WIN64)
//  Assuming Unix, which needs glext.h
#    include <GL/glext.h>
#  endif
#endif

#include <shader.h>
#include <GLFW/glfw3.h>
#include "config.h"

// Global variables have an underscore prefix.
GL::Program* _program;             //< GLSL program
GL::Program* _programDepth;        //< GLSL program for displaying a depth texture
glm::mat4    _projection;          //< Camera projection matrix

// Array objects - quad
GLuint       _vaoQuad;             //< Array object for the vertices
GLuint       _naoQuad;             //< Array object for the normals
GLuint       _taoQuad;             //< Array object for the texture coordintes

// Buffer objects - quad 
GLuint       _vertexBufferQuad;    //< Buffer object for the vertices
GLuint       _normalBufferQuad;    //< Buffer object for the normals
GLuint       _tcBufferQuad;        //< Buffer object for the texture coordinates

// Checkerboard texture
GLuint       _checkboard;          //< Texture object
int          _texWidth;            //< Width of the texture
int          _texHeight;           //< Height of the texture

// Shader file names
std::string  _vertexFile;          //< Name of the vertex shader file
std::string  _fragFile;            //< Name of the fragment shader file
std::string  _fragDepthFile;       //< Name of the fragment shader file

bool         _running;             //< true if the program is running, false if it is time to terminate
GLuint       _mvp;                 //< Location of the model, view, projection matrix in vertex shader
GLuint       _invTP;               //< Location of the inverse transpose of the MVP matrix
bool         _tracking;            //< True if mouse location is being tracked

// Data for quad
vector<vec4> _verticesQuad;        //< Vertex data
vector<vec4> _normalsQuad;         //< Normal data
vector<vec2> _tcQuad;              //< Texture coordinate data

// Window size
int          _winWidth;        //< Width of the window
int          _winHeight;       //< Height of the window

// Rotation
quat         _objRot;          //< Quaternion that describes the rotation of the object
vec2         _prevCurPos;      //< Previous cursor pos
float        _sensitivity;     //< Sensitivity to mouse motion

// FBO related handles and size
GLuint       _fbo;                 //< Frame buffer object handle
GLuint       _fboTextures[2];      //< FBO related textures
GLuint       _renderbuffer;        //< Render buffer handle
int          _fboWidth;            //< Width of the FBO textures
int          _fboHeight;           //< Height of the FBO textures

// Log file
std::ofstream _log;	//< Log file

enum FBOTextures
{
   DEPTH = 0,
   RGBA  = 1
};

void logException(const std::runtime_error& exception)
{
	std::cerr << exception.what() << std::endl;
	_log << exception.what() << std::endl;
}

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
      throw std::runtime_error(error);
   }
}

/**
 * Create an FBO that has an RGBA 32-bit floating point texture
 * and a texture for holding depth values
 */
void createFBO(void)
{
   GL_ERR_CHECK();
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
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _fboWidth, _fboHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      GL_ERR_CHECK();
      
      //------------------------------------------------------------------------------------------
      // Set up the RGBA texture for the rendered image
      //------------------------------------------------------------------------------------------
      glBindTexture(GL_TEXTURE_2D, _fboTextures[RGBA]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, _fboWidth, _fboHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      GL_ERR_CHECK();
      
      //------------------------------------------------------------------------------------------
      // Set up the render buffer
      //------------------------------------------------------------------------------------------
      glGenRenderbuffers(1, &_renderbuffer);
      glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
      glRenderbufferStorage(GL_RENDERBUFFER,  GL_RGBA32F, _fboWidth, _fboHeight);
      GL_ERR_CHECK();
      
      //------------------------------------------------------------------------------------------
      // Create the frame buffer object
      //------------------------------------------------------------------------------------------
      glGenFramebuffers(1, &_fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      GL_ERR_CHECK();
      
      //------------------------------------------------------------------------------------------
      // Attach textures and renderbuffer to FBO
      //------------------------------------------------------------------------------------------
      
      // Attach RGBA / Color texture to the FBO
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _fboTextures[RGBA], 0);
      
      // Attach depth texture to the FBO
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _fboTextures[DEPTH], 0);
      
      // Attach render buffer to the FBO
      //      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _renderbuffer);
      //      GL_ERR_CHECK();
      
      // Set the drawing buffer
      glDrawBuffer(GL_COLOR_ATTACHMENT0);
      
      // Set the reading buffer
      glReadBuffer(GL_NONE);
      GL_ERR_CHECK();
      
      fboStatus();

      // Return to default OpenGL state
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glBindTexture(GL_TEXTURE_2D, 0);
      // Set the drawing buffer
      glDrawBuffer(GL_BACK);
      // Set the reading buffer
      glReadBuffer(GL_BACK);
      GL_ERR_CHECK();
   } 
   catch (std::runtime_error exception)
   {
      logException(exception);
      terminate(EXIT_FAILURE);
   }
   GL_ERR_CHECK();
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
   try 
   {
      glewInit();
      // Initializing GLEW when using a core profile will result in the GL
      // being in an invalid state. This is because GLEW is trying to set
      // function pointers for deprecated functions. This next step
      // clears the GL error state
      while(glGetError());
      GL_ERR_CHECK();
      if(!GLEW_ARB_vertex_array_object)
      {
         std::cerr << "ARB_vertex_array_object not available." << std::endl; 
         terminate(EXIT_FAILURE);
      }
   } 
   catch(std::runtime_error exception)
   {
	   logException(exception);
   }
#endif
}

/**
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
void init(void)
{
   try
   {
      GL_ERR_CHECK();
      initGLEW();
      GL_ERR_CHECK();
      createFBO();	  
      
      // Create a checkerboard pattern
      _texWidth = 256;
      _texHeight = 256;
      
      vector<vec4> texels;
      texels.resize(_texWidth * _texHeight);
      for(int i = 0; i < _texWidth; i++ )
      {
         for(int j = 0; j < _texHeight; j++ )
         {
            GLubyte c = (((i & 0x8) == 0) ^ ((j & 0x8)  == 0)) * 255;
            int idx = j * _texWidth + i;
            texels.at(idx).r = c / (255.0f * 1.5f);
            texels.at(idx).g = 0;
            texels.at(idx).b = c / 255.0f;
            texels.at(idx).a = 1.0f;
         }
      }
      
      // Load the texture into OpenGL server
      GL_ERR_CHECK();
      glGenTextures(1, &_checkboard);
      glBindTexture(GL_TEXTURE_2D, _checkboard);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, &texels[0]);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glActiveTexture(GL_TEXTURE0);
      GL_ERR_CHECK();
      
      // Create a quad
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
      
      // Load the shader programs
      _vertexFile    = std::string(SOURCE_DIR) + "/vertex.c";
      _fragFile      = std::string(SOURCE_DIR) + "/fragment.c";
      _fragDepthFile = std::string(SOURCE_DIR) + "/fragmentDepth.c";
      
      _program      = new GL::Program(_vertexFile, _fragFile);
      _programDepth = new GL::Program(_vertexFile, _fragDepthFile);
      
      // Get vertex and color attribute locations
      //      getAttribLocations();
      
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
      glVertexAttribPointer(_program->getAttribLocation("vertex"), // Attribute location in the shader program
                            4,               // Number of components per attribute
                            GL_FLOAT,        // Data type of attribute
                            GL_FALSE,        // GL_TRUE: values are normalized or
                            0,               // Stride
                            0);              // Offset into currently bound array buffer for this data
      
      // Enable the generic vertex attribute array
      glEnableVertexAttribArray(_program->getAttribLocation("vertex"));
      
      // Set up normal attribute
      glGenBuffers(1, &_naoQuad);
      glBindBuffer(GL_ARRAY_BUFFER, _naoQuad);
      
      glBufferData(GL_ARRAY_BUFFER, _normalsQuad.size() * sizeof(glm::vec4),
                   &_normalsQuad[0], GL_STATIC_DRAW);
      
      glVertexAttribPointer(_program->getAttribLocation("normal"), 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_program->getAttribLocation("normal"));
      
      
      // Set up texture attribute
      glGenBuffers(1, &_taoQuad);
      glBindBuffer(GL_ARRAY_BUFFER, _taoQuad);
      
      glBufferData(GL_ARRAY_BUFFER, _tcQuad.size() * sizeof(glm::vec2),
                   &_tcQuad[0], GL_STATIC_DRAW);
      
      glVertexAttribPointer(_program->getAttribLocation("tc"), 2, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_program->getAttribLocation("tc"));
      
      // Set the clear color
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      
      // Set the depth clearing value
      glClearDepth(1.0f);
      
      // Enable depth test
      glEnable(GL_DEPTH_TEST);
      GL_ERR_CHECK();
   }
   catch (std::runtime_error exception)
   {
	  logException(exception);
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
   catch (std::runtime_error exception)
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
void resize(GLFWwindow* window, int width, int height)
{
   try
   {
      // Set the affine transform of (x,y) from normalized device coordinates to
      // window coordinates. In this case, (-1,1) -> (0, width) and (-1,1) -> (0, height)
      glViewport(0, 0, width, height);
      GL_ERR_CHECK();

      _winWidth = width;
      _winHeight = height;
   
      // Projection matrix
      _projection = glm::perspective(45.0f,                        // 45 degree field of view
                                  float(width) / float(height), // Ratio
                                  0.1f,                         // Near clip
                                  4000.0f);                     // Far clip
   }
   catch(std::runtime_error exception)
   {
      logException(exception);
      terminate(EXIT_FAILURE);
   }
}

/**
 *  Mouse click callback
 *
 *  @param window  pointer to the window being resized
 *  @param button that was clicked
 *  @param button state
 */
void mouseButton(GLFWwindow* window, int button, int action, int mods)
{
   if(button == GLFW_MOUSE_BUTTON_1)
   {
      if(action == GLFW_PRESS)
      {
         _tracking = true;
         double x, y;
         glfwGetCursorPos(window, &x, &y);
         _prevCurPos = vec2(x,y);
      }
      else
      {
         _tracking = false;
      }
   }
}

/**
 * Mouse movement callback
 */
void cursorPos(GLFWwindow* window, double x, double y)
{
   if(_tracking)
   {
      // Get the change in position
      vec2 curPos(x,y);
      vec2 delta = curPos - _prevCurPos;
      _prevCurPos = curPos;
      
      // This operation looks backwards, but movement in the x direction on
      // the rotates the model about the y-axis
      
      // Create Euler angle quaternion rotations based on cursor movement
      glm::quat yRot(vec3(0,                      delta.x * _sensitivity, 0));
      glm::quat xRot(vec3(delta.y * _sensitivity, 0,                      0));
      _objRot = glm::normalize(yRot * xRot * _objRot);
   }
}

/**
 * Keypress callback
 */
void keypress(GLFWwindow* window, int key, int scancode, int state, int mods)
{
   if(state == GLFW_PRESS)
   {
      switch(key)
      {
         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
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
void close(GLFWwindow* window)
{
   glfwSetWindowShouldClose(window, GL_TRUE);
}

/**
 * Draw the scene. Used to draw the scene into the fbo and
 * then into the default OpenGL framebuffer
 */
void drawScene(void)
{
   GL_ERR_CHECK();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glBindTexture(GL_TEXTURE_2D, _checkboard);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, _verticesQuad.size());
   GL_ERR_CHECK();
}
/**
 * Main loop
 * @param time    time elapsed in seconds since the start of the program
 */
int render(double time)
{
   try
   {
      // Camera matrix
      glm::mat4 view       = glm::lookAt(glm::vec3(0,0,2), // Camera position is at (0,0,2), in world space
                                         glm::vec3(0,0,0), // and looks at the origin
                                         glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                         );
      
      glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
      
      
      // Model matrix
      glm::mat4 model = glm::mat4_cast(_objRot);
      
      // Create  model, view, projection matrix
      glm::mat4 mvp        = _projection * view * translate * model; // Remember, matrix multiplication is the other way around
      
      // Use the shader program that was loaded, compiled and linked
      _program->bind();
      GL_ERR_CHECK();
      
      // Set the MVP uniform
      _program->setUniform("mvp", mvp);
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

      drawScene();
      
      //------------------------------------------------------------------------------------------
      // End draw FBO scene into an FBO
      //------------------------------------------------------------------------------------------
      
      //------------------------------------------------------------------------------------------
      // Draw the same scene into the default framebuffer
      //------------------------------------------------------------------------------------------
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      GL_ERR_CHECK();
      
      // Set the viewport for the default framebuffer
      glViewport(0, 0, _winWidth, _winHeight);
      
      // Clear the color and depth buffers
      glClearColor(0.3f, 0.5f, 0.9f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      drawScene();
      GL_ERR_CHECK();

      //------------------------------------------------------------------------------------------
      // End draw scene into default framebuffer
      //------------------------------------------------------------------------------------------
      
      //------------------------------------------------------------------------------------------
      // Draw a textured quad that contains the RGBA texture from the FBO
      //------------------------------------------------------------------------------------------
      float scaleFactor = 0.08f; //6125;
      glm::mat4 colorTrans 
         = glm::translate(glm::mat4(1.0f), glm::vec3(-0.8,0.7,0))
         * glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, 1.0f));
      
      glm::mat4 depthTrans 
         = glm::translate(glm::mat4(1.0f), glm::vec3(-0.6,0.7,0))
         * glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, 1.0f));
      
      mvp = _projection * view * colorTrans;
      
      GL_ERR_CHECK();

      // Set the MVP uniform
      _program->setUniform("mvp", mvp);
      
      glBindTexture(GL_TEXTURE_2D, _fboTextures[RGBA]);
      
      // Draw the triangles
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _verticesQuad.size());
      GL_ERR_CHECK();
      
      
      //------------------------------------------------------------------------------------------
      // Draw a textured quad that contains the depth texture from the FBO
      //------------------------------------------------------------------------------------------
      GL_ERR_CHECK();
      _programDepth->bind();
      mvp = _projection * view * depthTrans;
      
      // Set the MVP uniform
      _program->setUniform("mvp", mvp);
      
      glBindTexture(GL_TEXTURE_2D, _fboTextures[DEPTH]);
      
      // Draw the triangles
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _verticesQuad.size());
      GL_ERR_CHECK();
      //------------------------------------------------------------------------------------------
      // End drawing textured quads
      //------------------------------------------------------------------------------------------
      
      glBindTexture(GL_TEXTURE_2D, 0);
      GL_ERR_CHECK();
   } 
   catch (std::runtime_error exception)
   {
	  logException(exception);
      terminate(EXIT_FAILURE);
   }
   
   return GL_TRUE;
}

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
   int width = 1024; // Initial window width
   int height = 768; // Initial window height
   _sensitivity = float(M_PI) / 360.0f;
   
   // Open up the log file
   std::string logFile = std::string(PROJECT_BINARY_DIR) + "/log.txt";
   _log.open(logFile.c_str());

   GLFWwindow* window;
   
   // Initialize GLFW
   if(!glfwInit())
   {
      return EXIT_FAILURE;
   }
   
   // Request an OpenGL core profile context, without backwards compatibility
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_SAMPLES,               8);

#ifdef __APPLE__
   // This should never be needed and it is recommended to never set this bit,
   // but OS X requires it to be set.
   glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
#endif

   // Create a windowed mode window and its OpenGL context
   window = glfwCreateWindow(1024, 768, "FBO", NULL, NULL);
   if (!window)
   {
      fprintf(stderr, "Failed to open GLFW window\n");
      glfwTerminate();
      return EXIT_FAILURE;
   }
   
   glfwSetKeyCallback(window,         keypress);
   glfwSetMouseButtonCallback(window, mouseButton);
   glfwSetWindowSizeCallback(window,  resize);
   glfwSetWindowCloseCallback(window, close);
   glfwSetCursorPosCallback(window,   cursorPos);
   
   // Make the window's context current
   glfwMakeContextCurrent(window);
   
   printf("GL Version: %s\n", glGetString(GL_VERSION));
   
   init();
   resize(window, width, height);
   
   // Loop until the user closes the window
   while(!glfwWindowShouldClose(window))
   {
      // Render scene
      render(glfwGetTime());
      
      // Swap front and back buffers
      glfwSwapBuffers(window);
      
      // Poll for and process events
      glfwPollEvents();
   }
   
   glfwTerminate();
   
   terminate(EXIT_SUCCESS);
}
