//
// OpenGL 3.2 Texture Mapping example
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//

#include <iostream>
#include <fstream>
#include <vector>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace glm;
using std::vector;

#define _DEBUG

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
GL::Program* _program;         //< GLSL program
GLuint       _vao;             //< Array object for the vertices
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
vector<vec4> _vertexData;      //< Vertex data
vector<vec4> _normalData;      //< Normal data
vector<vec2> _tcData;          //< Texture coordinate data
std::string  _vertexFile;      //< Name of the vertex shader file
std::string  _fragFile;        //< Name of the fragment shader file
int          _winWidth;        //< Width of the window
int          _winHeight;       //< Height of the window
quat         _objRot;          //< Quaternion that describes the rotation of the object
vec2         _prevCurPos;      //< Previous cursor pos
float        _sensitivity;     //< Sensitivity to mouse motion

// Log file
std::ofstream _log;	//< Log file

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
   catch (std::runtime_error exception)
   {
      logException(exception);
      terminate(EXIT_FAILURE);
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

      // Set up the texture
      glGenTextures(1, &_texture);
      glBindTexture(GL_TEXTURE_2D, _texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_FLOAT, &texels[0]);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glActiveTexture(GL_TEXTURE0);
      GL_ERR_CHECK();
      
      _vertexData.push_back(glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f));
      _vertexData.push_back(glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f));
      _vertexData.push_back(glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f));
      _vertexData.push_back(glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f));
      
      _normalData.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalData.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalData.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
      _normalData.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

      _tcData.push_back(glm::vec2(0.0f, 0.0f));
      _tcData.push_back(glm::vec2(1.0f, 0.0f));
      _tcData.push_back(glm::vec2(0.0f, 1.0f));
      _tcData.push_back(glm::vec2(1.0f, 1.0f));
                        
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
      glGenBuffers(1, &_normalBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, _normalBuffer);
      
      glBufferData(GL_ARRAY_BUFFER, _normalData.size() * sizeof(glm::vec4),
                   &_normalData[0], GL_STATIC_DRAW);
      
      glVertexAttribPointer(_normalLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
      glEnableVertexAttribArray(_normalLocation);
      
      
      // Set up texture attribute
      glGenBuffers(1, &_tcBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, _tcBuffer);
      
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
   catch (std::runtime_error exception)
   {
      logException(exception);
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
   catch (std::runtime_error exception)
   {
      logException(exception);
      terminate(EXIT_FAILURE);
   }
}
/**
 * Window resize callback
 * 
 * @param window  pointer to the window being resized
 * @param width   the width of the window
 * @param height  the height of the window
 */
void resize(GLFWwindow* window, int width, int height)
{
   // Set the affine transform of (x,y) from normalized device coordinates to
   // window coordinates. In this case, (-1,1) -> (0, width) and (-1,1) -> (0, height)
   glViewport(0, 0, width, height);
   
   _winWidth = width;
   _winHeight = height;
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
      }
   }
}

/**
 * Main loop
 * @param time    time elapsed in seconds since the start of the program
 */
int render(double time)
{
   try
   {
      // Clear the color and depth buffers
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      // Projection matrix
      glm::mat4 projection = glm::perspective(45.0f,                                // 45 degree field of view
                                              float(_winWidth) / float(_winHeight), // Ratio
                                              0.1f,                                 // Near clip
                                              4000.0f);                             // Far clip
      // Camera matrix
      glm::mat4 view       = glm::lookAt(glm::vec3(0,0,2), // Camera position is at (0, 0, 2), in world space
                                         glm::vec3(0,0,0), // and looks at the origin
                                         glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                         );
      
      glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
      
      
      // Get vertex and color attribute locations
      getAttribLocations();

      // Model matrix 
      glm::mat4 model = glm::mat4_cast(_objRot);
      
      // Create  model, view, projection matrix
      glm::mat4 mvp   = projection * view * translate * model; // Remember, matrix multiplication is the other way around
      
      // Calculate the inverse transpose for use with normals
      glm::mat4 invTP = transpose(glm::inverse(mvp));
      
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
      glBindVertexArray(_vao);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, _vertexData.size());
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
 * Window close callback
 */
void close(GLFWwindow* window)
{
   glfwSetWindowShouldClose(window, GL_TRUE);
}

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
   int width = 1024; // Initial window width
   int height = 768; // Initial window height
   _sensitivity = M_PI / 360.0f;
   
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
#ifdef __APPLE__
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
   glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_SAMPLES,               8);
   
   // Create a windowed mode window and its OpenGL context
   window = glfwCreateWindow(1024, 768, "Triangle", NULL, NULL);
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
   
   resize(window, width, height);
   
   init();
   
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