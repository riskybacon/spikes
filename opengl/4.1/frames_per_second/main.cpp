//
// Basic shadow mapping using OpenGL 3.2
//
// Author: Jeff Bowles <jbowles@riskybacon.com>

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <utility>

#define _USE_MATH_DEFINES
#include <math.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // rotation, translation, scale, etc
#include <glm/gtc/type_ptr.hpp>         // value_ptr

using namespace glm;
using std::vector;
using std::string;
using std::make_pair;

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
#include <font_texture.h>
#include <GLFW/glfw3.h>
#include "config.h"

// Global variables have an underscore prefix.

// Array objects
enum VAO_OBJECTS
{
   QUAD_FLAT = 0,
   QUAD_SHADED,
   QUAD_TEXTURED,
   TORUS_SHADED,
   TORUS_POINTS,
   TORUS_LINES,
   TORUS_FLAT,
   NUM_VAO_OBJECTS
};

// Buffer objects
enum BUFFER_OBJECTS
{
   QUAD_POS = 0,
   QUAD_NORMAL,
   QUAD_TC,
   TORUS_POS,
   TORUS_NORMAL,
   TORUS_TC,
   TORUS_TRI_IDX,
   TORUS_LINES_IDX,
   NUM_BUFFER_OBJECTS
};

enum OBJ_TO_ROTATE
{
   ROTATE_OCCLUDER = 0,
   ROTATE_EYE,
   NUM_OBJ_TO_ROTATE
};

OBJ_TO_ROTATE _objToRotate;

std::vector<GLuint> _vao;          //< Vertex array object handles
std::vector<GLsizei> _vaoElements; //< Number of elements to draw in a VAO

GL::Program* _shadowProgram;       //< Shader program that performs shadow mapping
GL::Program* _flatProgram;         //< Shader program that performs no shading - all fragment get the same color
GL::Program* _texProgram;          //< Shader program that performs texture mapping - no shading

glm::mat4    _projection;          //< Camera projection matrix

std::vector<GLuint> _buffers;

// Shader file names
std::string  _shadowVertexFile;    //< Shadow mapping vertex shader
std::string  _shadowFragFile;      //< Shadow mapping fragment shader

std::string  _flatVertFile;        //< Flat vertex shader
std::string  _flatFragFile;        //< Flat fragment shader

std::string  _texVertFile;         //< Texture mapping vertex shader
std::string  _texFragFile;         //< Texture mapping fragment shader

bool         _tracking;            //< True if mouse location is being tracked

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
vec2         _texmapScale;         //< 1.0f / width and height of texture map

// Frames per second tracking
float        _fps;                 //< Frames per second
float        _numFrames;           //< Number of frames since last update
double       _lastFPSUpdate;       //< Time of last update in seconds
TextAlign    _align;               //< Text alignment
FontTexture* _fontTexture;         //< Font texture that has the FPS

glm::vec2    _dpi;                 //< Dots per inch for the screen.

// Log file
std::ofstream _log;	//< Log file

enum FBOTextures
{
   DEPTH = 0,
   NUM_FBO_TEXTURES
};

/**
 * Log an exception to stderr and to a log file
 * 
 * @param exception
 *    The exception to log
 */
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
   glDeleteVertexArrays(NUM_VAO_OBJECTS, &_vao[0]);
   glDeleteBuffers(NUM_BUFFER_OBJECTS, &_buffers[0]);
   glDeleteTextures(NUM_FBO_TEXTURES, &_fboTextures[0]);
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
      _fboWidth = 512;
      _fboHeight = 512;
      
      _texmapScale = vec2(1.0f / _fboWidth, 1.0f / _fboHeight);
      
      glGenTextures(NUM_FBO_TEXTURES, _fboTextures);
      GL_ERR_CHECK();
      
      for(int i = 0; i < NUM_FBO_TEXTURES; ++i)
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
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
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

      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);

      fboStatus();
      GL_ERR_CHECK();
      
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
 * Create torus vertex array object
 *
 * A torus is a circle of circles. numt defines the number
 * of divisions for the outer circle, numc defines the number
 * of divisions for the inner circles
 *
 * @param numc
 *   The number of circles
  * @param numt
 *   The number of divisions for each circle
 */
void createTorus(int numc, int numt, double radiusInner = 2, double radiusOuter = 2.3)
{
   int i, j;
   
   // Radius of the circles
   double radiusMiddle = abs((radiusOuter - radiusInner) * 0.5);
   // Distance from center of torus to the center of the circles
   double distToMiddle = radiusInner + radiusMiddle;
   
   // First step is to create a single circle and translate it out along
   // the x-axis
   vector<vec4> circlePos;
   circlePos.reserve(numc);

   vector<vec4> circleNormal;
   circleNormal.reserve(numc);
   
   for(int i = 0; i < numt; i++)
   {
      double theta = i * 2 * M_PI / numc;
      double y = sin(theta) * radiusMiddle;
      double x = cos(theta) * radiusMiddle + distToMiddle;
      
      vec4 pos4(x,y,0,1);
      vec3 normal3 = normalize(vec3(x,y,0));
      vec4 normal4(normal3.x, normal3.y, normal3.z, 0);
      
      circlePos.push_back(pos4);
      circleNormal.push_back(normal4);
   }

   // After the initial circle is created, make numc copies of the circle, but
   // rotate them about the y-axis
   vector<vec4> pos;
   vector<vec4> normals;
   vector<vec2> tc;
   pos.reserve(numc * numt);
   normals.reserve(numc * numt);
   tc.reserve(numc * numt);

   for(int j = 0; j < numc; j++)
   {
      float t = j / float(numc);
      
      quat rot(vec3(0.0f, t * 2 * M_PI, 0.0f));
      
      for(unsigned int i = 0; i < circlePos.size(); i++)
      {
         float s = i / float(circlePos.size());
         
         vec4 vPos = circlePos.at(i);
         vec4 vNormal = circleNormal.at(i);
         
         pos.push_back(rot * vPos);
         normals.push_back(normalize(rot * vNormal));
         tc.push_back(vec2(s,t));
      }
   }
   
   // Now that the points, normals and texture coordinates have been created, set up the element indices
   // to draw the points in the proper order. There are different orders for wireframe and triangle meshes.
   
   vector<GLuint> triIdx;
   vector<GLuint> linesIdx;
   
   for(i = 0; i < numc; i++)
   {
      int nextCol = i + 1;
      if(nextCol >= numc)
      {
         nextCol = 0;
      }

      for(j = 0; j <= numt; j++)
      {
         // Need to identify the four elements / indices that make
         // up the quad at this point

         
         int nextRow = j + 1;
         if(nextRow >= numt)
         {
            nextRow = 0;
         }

         GLuint ll = numc * i       + j;       // Lower left index
         GLuint ul = numc * i       + nextRow; // Upper left index
         GLuint lr = numc * nextCol + j;       // Lower right index
         GLuint ur = numc * nextCol + nextRow; // Upper right index
         
         triIdx.push_back(ul);
         triIdx.push_back(ll);
         triIdx.push_back(lr);
         
         triIdx.push_back(lr);
         triIdx.push_back(ur);
         triIdx.push_back(ul);
         
         
         linesIdx.push_back(ll);
         linesIdx.push_back(ul);

         linesIdx.push_back(ul);
         linesIdx.push_back(lr);

         linesIdx.push_back(lr);
         linesIdx.push_back(ll);
         
      }
   }
   
   //
   // Set up torus buffers for position, normals, texture coordinates, and element array indices
   // for wireframe and triangles
   //
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_POS]);
   glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), &pos[0], GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_NORMAL]);
   glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec4), &normals[0], GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_TC]);
   glBufferData(GL_ARRAY_BUFFER, tc.size() * sizeof(glm::vec2), &tc[0], GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[TORUS_TRI_IDX]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, triIdx.size() * sizeof(GLuint), &triIdx[0], GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[TORUS_LINES_IDX]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, linesIdx.size() * sizeof(GLuint), &linesIdx[0], GL_STATIC_DRAW);

   
   //
   // Point cloud torus
   //
   glBindVertexArray(_vao[TORUS_POINTS]);
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_POS]);
   glVertexAttribPointer(_flatProgram->getAttribLocation("vertex"), 4, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(_flatProgram->getAttribLocation("vertex"));
   _vaoElements[TORUS_POINTS] = pos.size();
   
   //
   // Wireframe torus
   //
   glBindVertexArray(_vao[TORUS_LINES]);

   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_POS]);
   glVertexAttribPointer(_flatProgram->getAttribLocation("vertex"), 4, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(_flatProgram->getAttribLocation("vertex"));
   
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[TORUS_LINES_IDX]);
   _vaoElements[TORUS_LINES] = linesIdx.size();
   

   //
   // Shaded torus
   //
   glBindVertexArray(_vao[TORUS_SHADED]);
   
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_POS]);
   glVertexAttribPointer(_shadowProgram->getAttribLocation("vertex"), 4, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(_shadowProgram->getAttribLocation("vertex"));
   
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_NORMAL]);
   glVertexAttribPointer(_shadowProgram->getAttribLocation("normal"), 4, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(_shadowProgram->getAttribLocation("normal"));
   
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_TC]);
   glVertexAttribPointer(_shadowProgram->getAttribLocation("tc"), 2, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(_shadowProgram->getAttribLocation("tc"));
   
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[TORUS_TRI_IDX]);
   _vaoElements[TORUS_SHADED] = triIdx.size();
   
   //
   // Flat shaded torus
   //
   glBindVertexArray(_vao[TORUS_FLAT]);
   
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[TORUS_POS]);
   glVertexAttribPointer(_flatProgram->getAttribLocation("vertex"), 4, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(_flatProgram->getAttribLocation("vertex"));
   
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[TORUS_TRI_IDX]);
   _vaoElements[TORUS_FLAT] = triIdx.size();
}

/**
 * Create quad / triangle strip square vertex array object
 */
void createQuad()
{
   GLint attribLoc;

   // Create a quad
   std::vector<glm::vec4> pos;
   std::vector<glm::vec4> normals;
   std::vector<glm::vec2> tc;
   
   pos.push_back(glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f));
   pos.push_back(glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f));
   pos.push_back(glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f));
   pos.push_back(glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f));
   
   normals.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
   normals.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
   normals.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
   normals.push_back(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
   
   tc.push_back(glm::vec2(0.0f, 0.0f));
   tc.push_back(glm::vec2(1.0f, 0.0f));
   tc.push_back(glm::vec2(0.0f, 1.0f));
   tc.push_back(glm::vec2(1.0f, 1.0f));

   // Load buffer data for positions, normals and texture coordinates
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_POS]);
   glBufferData(GL_ARRAY_BUFFER, pos.size()     * sizeof(glm::vec4), &pos[0],     GL_STATIC_DRAW);
   GL_ERR_CHECK();
   
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_NORMAL]);
   glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec4), &normals[0], GL_STATIC_DRAW);
   GL_ERR_CHECK();
   
   glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_TC]);
   glBufferData(GL_ARRAY_BUFFER, tc.size()      * sizeof(glm::vec2), &tc[0],      GL_STATIC_DRAW);
   GL_ERR_CHECK();

   //
   // Set up VAOs
   //
   std::vector<std::pair<VAO_OBJECTS, GL::Program*> > vaoList;
   vaoList.push_back(make_pair(QUAD_SHADED,   _shadowProgram));
   vaoList.push_back(make_pair(QUAD_TEXTURED, _texProgram));
   vaoList.push_back(make_pair(QUAD_FLAT,     _flatProgram));
   
   for(auto vao : vaoList)
   {
      //
      // Set up VAO for shaded quads, with texture coords and normals
      //
      glBindVertexArray(_vao[vao.first]);
   
      attribLoc = vao.second->getAttribLocation("vertex");
      if(attribLoc >= 0)
      {
         glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_POS]);
         glVertexAttribPointer(attribLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(attribLoc);
      }
   
      attribLoc = vao.second->getAttribLocation("normal");
      if(attribLoc >= 0)
      {
         glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_NORMAL]);
         glVertexAttribPointer(attribLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(attribLoc);
      }
   
      attribLoc = vao.second->getAttribLocation("tc");

      if(attribLoc >= 0)
      {
         glBindBuffer(GL_ARRAY_BUFFER, _buffers[QUAD_TC]);
         glVertexAttribPointer(attribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(attribLoc);
      }
      _vaoElements[vao.first] = pos.size();
   }
}

/**
 * Load font texture map
 */
void loadFontTexture()
{
   std::string font;
   font = std::string(FONT_DIR) + "/AnonymousPro-1.002.001/Anonymous Pro.ttf";
   //   font = std::string(FONT_DIR) + "/Minecraftia.ttf";
   font = std::string(FONT_DIR) + "/Lato-Regular.ttf";
   std::string text = "fps: calculating...";
   float pointSize = 18.0f;
   vec4 fgColor(1,1,0,1);
   _align = TEXT_ALIGN_CENTER;
   _fontTexture = new FontTexture(font, text, pointSize, fgColor, _align, _dpi);
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
      loadFontTexture();
      
      _occluderRot = quat(vec3(0, 0, 0));
      _receiverRot = quat(vec3(M_PI / 2, 0, 0));
      
      // Load the shader programs
      _shadowVertexFile = std::string(SOURCE_DIR) + "/shadow.vsh";
      _shadowFragFile   = std::string(SOURCE_DIR) + "/shadow.fsh";
      
      _flatVertFile     = std::string(SOURCE_DIR) + "/flat.vsh";
      _flatFragFile     = std::string(SOURCE_DIR) + "/flat.fsh";
      
      _texVertFile      = std::string(SOURCE_DIR) + "/texture.vsh";
      _texFragFile      = std::string(SOURCE_DIR) + "/texture.fsh";
      
      _shadowProgram = new GL::Program(_shadowVertexFile, _shadowFragFile);
      _flatProgram   = new GL::Program(_flatVertFile,     _flatFragFile);
      _texProgram    = new GL::Program(_texVertFile, _texFragFile);
      
      // Generate handles for vertex array objects
      _vao.resize(NUM_VAO_OBJECTS, 0);
      _vaoElements.resize(NUM_VAO_OBJECTS, 0);
      glGenVertexArrays(NUM_VAO_OBJECTS, &_vao[0]);
      
      // Generate handles for buffers
      _buffers.resize(NUM_BUFFER_OBJECTS, 0);
      glGenBuffers(NUM_BUFFER_OBJECTS, &_buffers[0]);
      
      createQuad();
      createTorus(50,50,1, 1.5);
      
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
            yRot         = quat(eulerY * _eyeRot);
            xRot         = quat(eulerX * _eyeRot);
            _occluderRot = normalize(yRot * xRot * _occluderRot);
            break;
            
         case ROTATE_EYE:
            yRot    = quat(eulerY);
            xRot    = quat(eulerX);
            _eyeRot = normalize(yRot * xRot * _eyeRot);
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
         case GLFW_KEY_SPACE:
            _objToRotate = _objToRotate == ROTATE_OCCLUDER ? ROTATE_EYE : ROTATE_OCCLUDER;
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
 * Update the frames per second counter
 *
 * @param time
 *    time elapsed in seconds
 */
void updateFPS(double time)
{
   _numFrames++;
   
   // Update frames per second every 5 seconds
   if(_lastFPSUpdate + 5 < time)
   {
      _fps = float(_numFrames) / (time - _lastFPSUpdate);
      _numFrames = 0;
      _lastFPSUpdate = time;
   
      std::stringstream ss;
      int precision = (int) _fps;
      
      std::stringstream wholeDigits;
      wholeDigits << (int) _fps;
      
      precision = wholeDigits.str().length() + 1;
      
      ss << "fps: " << std::setprecision(precision) << " " << _fps;
      
      int len = ss.str().length();
      
      if(ss.str()[len - 2] != '.')
      {
         ss << ".0";
      }
      
      _fontTexture->setText(ss.str());
      _fontTexture->update();
   }

}

/**
 * Draw frames per second
 */
void drawSceneInfo(double time)
{
   updateFPS(time);
   
   // The texture size in terms of a percentage of window width and height
   glm::vec2 texSize = vec2(_fontTexture->getSize().x / _winWidth, _fontTexture->getSize().y / _winHeight);
   
   // Move the texture down to the lower left, but then move it back up based on the
   // size of the texture map
   glm::vec2 textTrans = texSize - vec2(0.99,0.99);
   
   glm::mat4 mvp = glm::translate(glm::mat4(), vec3(textTrans,0)) *
   glm::scale(glm::mat4(), vec3(texSize.x, texSize.y, 1));
   
   // Use the shader program that was loaded, compiled and linked
   GL::Program* program = _texProgram;
   
   program->bind();
   GL_ERR_CHECK();
   
   // Set the MVP uniform
   program->setUniform("mvp", mvp);
   
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, _fontTexture->getID());
   GL_ERR_CHECK();
   
   // Draw the triangles
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glBindVertexArray(_vao[QUAD_TEXTURED]);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //_vaoElements[QUAD_SHADED]);
   glDisable(GL_BLEND);
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
      glClearDepth(1.0f);
      mat4 translate;
      mat4 scale;
      mat4 mvp;
      mat4 rot;
      vec4 lightPos = vec4(0, 10, 0, 1);
      mat4 toShadowTex0;
      mat4 toShadowTex1;
      mat4 modelOccluder;
      mat4 modelReceiver;

      // Matrix that maps from [-1, 1] -> [0,1], which maps from clip space to texture map space
      mat4 clipToTexture = glm::scale(glm::translate(mat4(1), vec3(0.5, 0.5, 0.5)), vec3(0.5, 0.5, 0.5));

      //----------------------------------------------------------------------------------------------------
      // Draw depth pass from light's point of view.
      //----------------------------------------------------------------------------------------------------
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
      glViewport(0, 0, _fboWidth, _fboHeight);
      
      // Clear the framebuffer
      glClearColor(0, 0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      // Set up the light's view and projection matrices
      glm::mat4 lightView = glm::lookAt(vec3(lightPos.x, lightPos.y, lightPos.z), vec3(0, 0, 0), vec3(0, 0, 1));
      glm::mat4 lightProj = glm::perspective(90.0f,                        // 45 degree field of view
                                             float(_winWidth) / float(_winHeight), // Ratio
                                             0.1f,                         // Near clip
                                             1000.0f);                     // Far clip
      
      //----------------------------------------
      // Draw the occluding surface, flat shaded
      // all we care about is the depth
      //----------------------------------------
      
      // Set up model, view, projection matrix for occluding surface
      rot           = glm::mat4_cast(_occluderRot);
      translate     = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 0.0f));
      modelOccluder = translate * rot;
      mvp           = lightProj * lightView * modelOccluder;
      
      // Need to keep the mvp for this surface from the light's point of view. This will be used
      // in the render pass where the shadows are drawn
      toShadowTex0 = clipToTexture * mvp;
      
      // Bind the flat shader program. No need for a fancy shader on this pass, just need the depth
      _flatProgram->bind();
      _flatProgram->setUniform("mvp",      mvp);
      
      // Draw the occluding surface
      glBindVertexArray(_vao[TORUS_FLAT]);
      glDrawElements(GL_TRIANGLES, _vaoElements[TORUS_FLAT], GL_UNSIGNED_INT, NULL);
      GL_ERR_CHECK();
      
      // Set up modle, view, projection matrix for the receiving surface
      rot           = glm::mat4_cast(_receiverRot);
      translate     = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
      scale         = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 1.0f));
      modelReceiver = translate * rot * scale;
      mvp           = lightProj * lightView * modelReceiver;

      // Need to keep the mvp for this surface from the light's point of view. This will be used
      // in the render pass where the shadows are drawn
      toShadowTex1 = clipToTexture * mvp;
      
      _flatProgram->setUniform("mvp",      mvp);
      
      // Draw occluding surface. Use the same vertex array object as the previous surface - they're both
      // the same shape, just different position, rotation and scale
      glBindVertexArray(_vao[QUAD_FLAT]);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _vaoElements[QUAD_FLAT]);
      GL_ERR_CHECK();

      //----------------------------------------------------------------------------------------------------
      // Draw pass from camera's point of view
      //----------------------------------------------------------------------------------------------------
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0, 0, _winWidth, _winHeight);
      glClearColor(0.3f, 0.4f, 0.95f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      GL_ERR_CHECK();
      
      // view = lookat matrix * eye rotation matrix
      glm::mat4 view = glm::lookAt(vec3(0, 0, 10), vec3(0, 0, 0), vec3(0, 1, 0)) * glm::mat4_cast(_eyeRot);
      
      lightPos = vec4(10, 10, -10, 1);
      
      // Set up model, view, projection matrix for occluding surface
      mvp        = _projection * view * modelOccluder;
      
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _fboTextures[DEPTH]);
      glGenerateMipmap(GL_TEXTURE_2D);
      
      // Bind the shader program that will draw the shadows and do some simple shading
      _shadowProgram->bind();
      _shadowProgram->setUniform("mvp",           mvp);
      _shadowProgram->setUniform("depthMap",      0);
      _shadowProgram->setUniform("toShadowTex",   toShadowTex0);
      _shadowProgram->setUniform("texmapScale",   _texmapScale);
      
      glBindVertexArray(_vao[TORUS_SHADED]);
      glDrawElements(GL_TRIANGLES, _vaoElements[TORUS_SHADED], GL_UNSIGNED_INT, NULL);

      GL_ERR_CHECK();
      
      mvp        = _projection * view * modelReceiver;

      _shadowProgram->bind();
      _shadowProgram->setUniform("mvp",         mvp);
      _shadowProgram->setUniform("toShadowTex", toShadowTex1);
      
      // Draw the receiving surface
      glBindVertexArray(_vao[QUAD_SHADED]);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _vaoElements[QUAD_SHADED]);
      GL_ERR_CHECK();

      drawSceneInfo(time);
   }
   catch (std::runtime_error exception)
   {
	   logException(exception);
      terminate(EXIT_FAILURE);
   }
   
   return GL_TRUE;
}

/**
 * Get monitor characteristics
 */
void getMonitorMetrics()
{
   GLFWmonitor* monitor = glfwGetPrimaryMonitor();
   const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
   int width;
   int height;
   
   glfwGetMonitorPhysicalSize(monitor, &width, &height);
   
   // Get the dots per inch. The width,height variables are in mm, so convert to inches
   _dpi = vec2(vidmode->width * 25.4 / width , vidmode->height * 25.4 / height);
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
   _fps = 0;
   _lastFPSUpdate = 0;
   // Open up the log file
   std::string logFile = std::string(PROJECT_BINARY_DIR) + "/log.txt";
   _log.open(logFile.c_str());
   
   GLFWwindow* window;
   
   // Initialize GLFW
   if(!glfwInit())
   {
      return EXIT_FAILURE;
   }
   glfwSwapInterval(0);
   
   getMonitorMetrics();
   
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
   // but OS X requires it to be set to get a core profile.
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
   
   // Create a windowed mode window and its OpenGL context
   window = glfwCreateWindow(1024, 768, "FBO", NULL, NULL);
   if(!window)
   {
      std::cerr << "Failed to open GLFW window" << std::endl;
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