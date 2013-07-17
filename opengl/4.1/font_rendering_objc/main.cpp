//
// OpenGL 3.2 Font rendering with CoreText / Objective C++
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <CoreFoundation/CoreFoundation.h>

CFStringRef ref;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "font_texture.h"
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
#include <config.h>
#include "font_texture.h"

// Global variables have an underscore prefix.
GL::Program* _program;         //< GLSL program
GLuint       _vao;             //< Array object for the vertices
GLuint       _vertexBuffer;    //< Buffer object for the vertices
GLuint       _normalBuffer;    //< Buffer object for the normals
GLuint       _tcBuffer;        //< Buffer object for the texture coordinates
bool         _running;         //< true if the program is running, false if it is time to terminate
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
FontTexture* _fontTexture;
TextAlign    _align;

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
 * Load checkerboard texture map
 */
void loadTexture()
{
   std::string font = "Menlo";
   std::string text = "Time:";
   float pointSize = 17.0f;
   vec4 fgColor(1,1,0,1);
   vec4 bgColor(0,0,0,0);
   _align = TEXT_ALIGN_CENTER;
   _fontTexture = new FontTexture(font, text, pointSize, fgColor, bgColor, _align);
}

/**
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
void init(void)
{
   GLint attribLoc;
   
   try
   {
      initGLEW();
      
      loadTexture();

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
                        
      _vertexFile = std::string(SOURCE_DIR) + "/texture.vsh";
      _fragFile   = std::string(SOURCE_DIR) + "/texture.fsh";
      
      _program = new GL::Program(_vertexFile, _fragFile);
      
      // Generate a single handle for a vertex array. Only one vertex
      // array is needed
      glGenVertexArrays(1, &_vao);
      
      // Bind that vertex array
      glBindVertexArray(_vao);
      
      attribLoc = _program->getAttribLocation("vertex");
      if(attribLoc >= 0)
      {
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
         glVertexAttribPointer(attribLoc,       // Attribute location in the shader program
                               4,               // Number of components per attribute
                               GL_FLOAT,        // Data type of attribute
                               GL_FALSE,        // GL_TRUE: values are normalized or
                               0,               // Stride
                               0);              // Offset into currently bound array buffer for this data
         
         // Enable the generic vertex attribute array
         glEnableVertexAttribArray(attribLoc);
      }
      
      attribLoc = _program->getAttribLocation("normal");
      if(attribLoc >= 0)
      {
         // Set up normal attribute
         glGenBuffers(1, &_normalBuffer);
         glBindBuffer(GL_ARRAY_BUFFER, _normalBuffer);
         
         glBufferData(GL_ARRAY_BUFFER, _normalData.size() * sizeof(glm::vec4),
                      &_normalData[0], GL_STATIC_DRAW);
         
         glVertexAttribPointer(attribLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(attribLoc);
      }
      
      
      attribLoc = _program->getAttribLocation("tc");
      if(attribLoc >= 0)
      {
         // Set up texture attribute
         glGenBuffers(1, &_tcBuffer);
         glBindBuffer(GL_ARRAY_BUFFER, _tcBuffer);
         
         glBufferData(GL_ARRAY_BUFFER, _tcData.size() * sizeof(glm::vec2),
                      &_tcData[0], GL_STATIC_DRAW);
         
         glVertexAttribPointer(attribLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(attribLoc);
      }
      
      // Set the clear color
      glClearColor(0.3f, 0.1f, 0.1f, 0.0f);
      
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
      std::stringstream ss;
      int precision = time;

      
      std::stringstream wholeDigits;
      wholeDigits << (int) time;
      
      precision = wholeDigits.str().length() + 1;

      ss << "Time: " << std::setprecision(precision) << " " << time;
      
      int len = ss.str().length();
      
      if(ss.str()[len - 2] != '.')
      {
         ss << ".0";
      }
      
      //      std::cout << ss.str() << std::endl;
      _fontTexture->setText(ss.str());
      _fontTexture->update();
      
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
      

      
      // Model matrix 
      glm::mat4 model = glm::mat4_cast(_objRot);
      
      // Create  model, view, projection matrix
      glm::mat4 mvp   = projection * view * translate * model; // Remember, matrix multiplication is the other way around
      // The texture size in terms of a percentage of window width and height
      glm::vec2 texSize = vec2(_fontTexture->getSize().x / _winWidth, _fontTexture->getSize().y / _winHeight);

      // Half of the texture size - needed if text is centered
      glm::vec2 texSizeHalf = texSize * 0.5f;
      glm::vec2 lowerLeft;
      
      switch(_align)
      {
         case TEXT_ALIGN_CENTER:
            lowerLeft = texSizeHalf - vec2(1,1) + texSizeHalf;
            break;
            
         case TEXT_ALIGN_LEFT:
            lowerLeft = vec2(-1 + texSize.x, -1 + texSize.y);
            break;
            
         default:
            lowerLeft = vec2(0,0);
            break;
      }
      
      
      mvp = glm::translate(glm::mat4(), vec3(lowerLeft,0)) *
           glm::scale(glm::mat4(), vec3(texSize.x, texSize.y, 1));


      // Use the shader program that was loaded, compiled and linked
      _program->bind();
      GL_ERR_CHECK();
      
      // Set the MVP uniform
      _program->setUniform("mvp", mvp);
      _program->setUniform("tex", 0);
      
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _fontTexture->getID());
      
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
   int height = 1024; // Initial window height
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
   
   // Create a windowed mode window and its OpenGL context
   window = glfwCreateWindow(width, height, "Text Rendering with CoreText", NULL, NULL);
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