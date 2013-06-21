//
// Basic shadow mapping using OpenGL 3.2
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
using std::string;

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
GL::Program* _shadowProgram;
GL::Program* _flatProgram;
GL::Program* _texProgram;

glm::mat4    _projection;          //< Camera projection matrix

// Array objects - quad
enum VAO_OBJECTS
{
   FLAT_QUAD = 0,
   SHADED_QUAD,
   TEXTURED_QUAD,
   NUM_VAO
};

std::vector<GLuint> _vao; //< Vertex array object handles

// Buffer objects - quad
enum BUFFER_OBJECTS
{
   QUAD_POS = 0,
   QUAD_NORMAL,
   QUAD_TC,
   NUM_BUFFER
};

enum OBJ_TO_ROTATE
{
   ROTATE_OCCLUDER = 0,
   ROTATE_EYE,
   NUM_OBJ_TO_ROTATE
};

OBJ_TO_ROTATE _objToRotate;

std::vector<GLuint> _buffers;

// Shader file names
std::string  _shadowVertexFile;    //< Name of the vertex shader file
std::string  _shadowFragFile;      //< Name of the fragment shader file
std::string  _fragDepthFile;       //< Name of the fragment shader file

std::string  _flatVertFile;        //< Vertex shader filename for flat shading
std::string  _flatFragFile;        //< Fragment shader filename for flat shading

std::string  _texVertFile;
std::string  _texFragFile;

bool         _running;             //< true if the program is running, false if it is time to terminate
bool         _tracking;            //< True if mouse location is being tracked

// Data for quad
vector<vec4> _posQuad;             //< Position data
vector<vec4> _normalsQuad;         //< Normal data
vector<vec2> _tcQuad;              //< Texture coordinate data

// Window size
int          _winWidth;            //< Width of the window
int          _winHeight;           //< Height of the window

// Rotation
quat         _occluderRot;         //< Quaternion that describes the rotation of occluding object
quat         _receiverRot;         //< Rotation for the plane that receives the shadow
quat         _eyeRot;              //< Eye rotation
vec2         _prevCurPos;          //< Previous cursor pos
float        _sensitivity;         //< Sensitivity to mouse motion

vec4         _eye;                 //< Eye position;

// FBO related handles and size
GLuint       _fbo;                 //< Frame buffer object handle
GLuint       _fboTextures[2];      //< FBO related textures
GLuint       _renderbuffer;        //< Render buffer handle
int          _fboWidth;            //< Width of the FBO textures
int          _fboHeight;           //< Height of the FBO textures

bool         _viewFromLight;

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
   glDeleteVertexArrays(NUM_VAO, &_vao[0]);
   glDeleteBuffers(NUM_BUFFER, &_buffers[0]);
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
      // Create the frame buffer object
      //------------------------------------------------------------------------------------------
      glGenFramebuffers(1, &_fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      GL_ERR_CHECK();
      
      //------------------------------------------------------------------------------------------
      // Attach textures and renderbuffer to FBO
      //------------------------------------------------------------------------------------------

      // Attach depth texture to the FBO
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _fboTextures[DEPTH], 0);
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
      initGLEW();
      createFBO();

      _occluderRot = quat(vec3(M_PI / 2, 0, 0));
      _receiverRot = quat(vec3(M_PI / 2, 0, 0));
      
      // Create a quad
      _posQuad.push_back(glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f));
      _posQuad.push_back(glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f));
      _posQuad.push_back(glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f));
      _posQuad.push_back(glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f));
      
      _normalsQuad.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
      _normalsQuad.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
      _normalsQuad.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
      _normalsQuad.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
      
      _tcQuad.push_back(glm::vec2(0.0f, 0.0f));
      _tcQuad.push_back(glm::vec2(1.0f, 0.0f));
      _tcQuad.push_back(glm::vec2(0.0f, 1.0f));
      _tcQuad.push_back(glm::vec2(1.0f, 1.0f));
      
      // Load the shader programs
      _shadowVertexFile = std::string(SOURCE_DIR) + "/shadow_vertex.c";
      _shadowFragFile   = std::string(SOURCE_DIR) + "/shadow_fragment.c";
      
      _flatVertFile     = std::string(SOURCE_DIR) + "/flat_vertex.c";
      _flatFragFile     = std::string(SOURCE_DIR) + "/flat_fragment.c";
      
      _texVertFile      = std::string(SOURCE_DIR) + "/tex_vertex.c";
      _texFragFile      = std::string(SOURCE_DIR) + "/tex_fragment.c";
      
      _shadowProgram = new GL::Program(_shadowVertexFile, _shadowFragFile);
      _flatProgram   = new GL::Program(_flatVertFile,     _flatFragFile);
      _texProgram   = new GL::Program(_texVertFile,      _texFragFile);
      
      // Generate handles for vertex array objects
      _vao.resize(NUM_VAO, 0);
      glGenVertexArrays(NUM_VAO, &_vao[0]);

      // Generate handles for buffers
      _buffers.resize(NUM_BUFFER, 0);
      glGenBuffers(NUM_BUFFER, &_buffers[0]);
      
      
      // Load buffer data for positions, normals and texture coordinates
      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_POS]);
      glBufferData(GL_ARRAY_BUFFER, _posQuad.size()     * sizeof(glm::vec4), &_posQuad[0],     GL_STATIC_DRAW);
      GL_ERR_CHECK();

      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_NORMAL]);
      glBufferData(GL_ARRAY_BUFFER, _normalsQuad.size() * sizeof(glm::vec4), &_normalsQuad[0], GL_STATIC_DRAW);
      GL_ERR_CHECK();

      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_TC]);
      glBufferData(GL_ARRAY_BUFFER, _tcQuad.size()      * sizeof(glm::vec2), &_tcQuad[0],      GL_STATIC_DRAW);
      GL_ERR_CHECK();

      // Set up VAO for flat shaded quads, no texture coords
      glBindVertexArray(_vao[FLAT_QUAD]);

      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_POS]);
      glVertexAttribPointer(_flatProgram->getAttribLocation("vertex"), 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_flatProgram->getAttribLocation("vertex"));

      // Set up VAO for shaded quads, with texture coords
      glBindVertexArray(_vao[SHADED_QUAD]);

      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_POS]);
      glVertexAttribPointer(_shadowProgram->getAttribLocation("vertex"), 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_shadowProgram->getAttribLocation("vertex"));

      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_NORMAL]);
      glVertexAttribPointer(_shadowProgram->getAttribLocation("normal"), 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_shadowProgram->getAttribLocation("normal"));

      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_TC]);
      glVertexAttribPointer(_shadowProgram->getAttribLocation("tc"), 2, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_shadowProgram->getAttribLocation("tc"));

      
      // Set up VAO for shaded quads, with texture coords
      glBindVertexArray(_vao[TEXTURED_QUAD]);
      
      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_POS]);
      glVertexAttribPointer(_texProgram->getAttribLocation("vertex"), 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_texProgram->getAttribLocation("vertex"));
      
      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_NORMAL]);
      glVertexAttribPointer(_texProgram->getAttribLocation("normal"), 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_texProgram->getAttribLocation("normal"));
      
      glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_TC]);
      glVertexAttribPointer(_texProgram->getAttribLocation("tc"), 2, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_texProgram->getAttribLocation("tc"));

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
      GL::Program* newProgram;
      newProgram = new GL::Program(_flatVertFile, _flatFragFile);
      delete _flatProgram;
      _flatProgram = newProgram;

      newProgram = new GL::Program(_shadowVertexFile, _shadowFragFile);
      delete _shadowProgram;
      _shadowProgram = newProgram;

      newProgram = new GL::Program(_texVertFile, _texFragFile);
      delete _shadowProgram;
      _texProgram = newProgram;
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
                                  100.0f);                     // Far clip
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

      
      vec3 eulerY = vec3(0, 1, 0) * delta.x * _sensitivity;
      vec3 eulerX = vec3(1, 0, 0) * delta.y * _sensitivity;
      
      glm::quat xRot;
      glm::quat yRot;
      
      // This operation looks backwards, but movement in the x direction on
      // the rotates the model about the y-axis
      
      // Create Euler angle quaternion rotations based on cursor movement
      switch(_objToRotate)
      {
         case ROTATE_OCCLUDER:
            yRot = glm::quat(eulerY * _eyeRot);
            xRot = glm::quat(eulerX * _eyeRot);
            _occluderRot = glm::normalize(yRot * xRot * _occluderRot);
            break;
            
         case ROTATE_EYE:
            yRot = glm::quat(eulerY);
            xRot = glm::quat(eulerX);
            _eyeRot      = glm::normalize(yRot * xRot * _eyeRot);
         default:
            break;
      }
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
         case GLFW_KEY_SPACE:
            _objToRotate = _objToRotate == ROTATE_OCCLUDER ? ROTATE_EYE : ROTATE_OCCLUDER;
            break;
            
         case 'L':
         case 'l':
            _viewFromLight = !_viewFromLight;
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
 * Main loop
 * @param time    time elapsed in seconds since the start of the program
 */
int render(double time)
{
   try
   {
      glClearDepth(1.0f);

      mat4 translate;
      mat4 scale;
      mat4 mvp;
      mat4 rot;
      mat4 invTP;
      vec4 lightPos = vec4(0, 10, 0, 1);
      mat4 toShadowTex0;
      mat4 toShadowTex1;
      mat4 clipToTexture = glm::scale(glm::translate(mat4(1), vec3(0.5, 0.5, 0.5)), vec3(0.5, 0.5, 0.5));
      
      if(!_viewFromLight)
      {
         glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
         glViewport(0, 0, _fboWidth, _fboHeight);
      }
      
      glClearColor(0, 0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      glm::mat4 lightView = glm::lookAt(vec3(lightPos.x, lightPos.y, lightPos.z), vec3(0, 0, 0), vec3(0, 0, 1));
      glm::mat4 lightProj = glm::perspective(30.0f,                        // 45 degree field of view
                                             float(_winWidth) / float(_winHeight), // Ratio
                                             0.1f,                         // Near clip
                                             100.0f);                     // Far clip

      rot        = glm::mat4_cast(_occluderRot);
      translate  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
      mvp        = lightProj * lightView * translate * rot;
      toShadowTex0 = clipToTexture * mvp;
      
      _flatProgram->bind();
      _flatProgram->setUniform("mvp",      mvp);
      
      glBindVertexArray(_vao[FLAT_QUAD]);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _posQuad.size());
      
      rot          = glm::mat4_cast(_receiverRot);
      translate    = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
      scale        = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 1.0f));

      mvp          = lightProj * lightView * translate * rot * scale;
      toShadowTex1 = clipToTexture * mvp;
      
      _shadowProgram->setUniform("mvp",      mvp);
      
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _posQuad.size());

      if(!_viewFromLight)
      {
         glBindFramebuffer(GL_FRAMEBUFFER, 0);
         glViewport(0, 0, _winWidth, _winHeight);
         glClearColor(0.3f, 0.4f, 0.95f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         
         glm::mat4 view = glm::lookAt(vec3(0, 0, 10), vec3(0, 0, 0), vec3(0, 1, 0)) * glm::mat4_cast(_eyeRot);
         
         lightPos = view * vec4(10, 10, -10, 1);
         
         rot        = glm::mat4_cast(_occluderRot);
         translate  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
         mvp        = _projection * view * translate * rot;
         invTP      = transpose(inverse(mvp));
         
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, _fboTextures[DEPTH]);
         
         _shadowProgram->bind();
         _shadowProgram->setUniform("mvp",      mvp);
         _shadowProgram->setUniform("invTP",    invTP);
         _shadowProgram->setUniform("lightPos", lightPos);
         _shadowProgram->setUniform("depthMap", 0);
         _shadowProgram->setUniform("toShadowTex", toShadowTex0);
         
         glBindVertexArray(_vao[SHADED_QUAD]);
         glDrawArrays(GL_TRIANGLE_STRIP, 0, _posQuad.size());
         
         rot        = glm::mat4_cast(_receiverRot);
         translate  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
         scale      = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 1.0f));
         mvp        = _projection * view * translate * rot * scale;
         invTP      = transpose(inverse(mvp));
         
         _shadowProgram->setUniform("mvp",      mvp);
         _shadowProgram->setUniform("invTP",    invTP);
         _shadowProgram->setUniform("toShadowTex", toShadowTex1);
         
         glDrawArrays(GL_TRIANGLE_STRIP, 0, _posQuad.size());
         GL_ERR_CHECK();

         //------------------------------------------------------------------------------------------
         // Draw a textured quad that contains the depth texture from the FBO
         //------------------------------------------------------------------------------------------
         float scaleFactor = 0.2f; //6125;

         mat4 depthTrans = glm::translate(glm::mat4(1.0f), glm::vec3(-0.75,0.75,0))
                         * glm::scale(glm::mat4(1.0f),     glm::vec3(scaleFactor, scaleFactor, 1.0f));
         
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, _fboTextures[DEPTH]);

         _texProgram->bind();
         _texProgram->setUniform("mvp", depthTrans);
         _texProgram->setUniform("tex", 0);
         
         
         glBindVertexArray(_vao[TEXTURED_QUAD]);
         glDrawArrays(GL_TRIANGLE_STRIP, 0, _posQuad.size());
         GL_ERR_CHECK();
      }
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
   _objToRotate = ROTATE_OCCLUDER;
   _eye = vec4(0.0f, 0.0f, 2.0f, 1.0f);
   _viewFromLight = false;
   
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
   glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_SAMPLES,               8);
   glfwWindowHint(GLFW_RED_BITS,              32);
   glfwWindowHint(GLFW_GREEN_BITS,            32);
   glfwWindowHint(GLFW_BLUE_BITS,             32);
   glfwWindowHint(GLFW_ALPHA_BITS,            32);
   
#ifdef __APPLE__
   // This should never be needed and it is recommended to never set this bit,
   // but OS X requires it to be set.
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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
