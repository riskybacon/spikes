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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Include this file before glfw.h
#include "platform_specific.h"
#include <GL/glfw.h>

#include "trackball.h"

// Global variables have an underscore prefix.
GLuint     _program;        //< Shader program handle
GLuint     _vao;            //< Vertex array object for the vertices
GLuint     _cao;            //< Vertex array object for the colors
GLuint     _iao;            //< Array object for the indices
GLuint     _vertices;       //< Vertex buffer object for the vertices
GLuint     _colors;         //< Vertex buffer object for the colors
GLuint     _indices;        //< Buffer object for the vertex indices
GLint      _vertexLocation; //< Location of the vertex attribute in the shader program
GLint      _colorLocation;  //< Location of the color attribute in the shader program
bool       _running;        //< true if the program is running, false if it is time to terminate
GLuint     _mvp;            //< Location of the model, view, projection matrix in vertex shader
bool       _tracking;       //< True if mouse location is being tracked
Trackball* _trackball;      //< Pointer to virtual trackball

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
      glDeleteProgram(_program);
   }

   glfwTerminate();

   exit(exitCode);
}

/**
 * Creates a string by reading a text file.
 *
 * @param filename	The name of the file
 * @return			A string that contains the contents of the file
 */
std::string readTextFile(const std::string& filename)
{
   std::ifstream infile(filename.c_str()); // File stream
   std::string source;                     // Text file string
   std::string line;                       // A line in the file

   // Make sure the file could be opened
   if(!infile.is_open())
   {
      std::cerr << "Could not open file: " << filename << std::endl;
      terminate(EXIT_FAILURE);
   }

   // Read in the source one line at a time, then append it
   // to the source string. Not efficient.
   while(infile.good())
   {
	  getline(infile, line);
	  source = source + line + "\n";
   }

   infile.close();
   return source;
}

/**
 * Check the compile status of a shader
 *
 * @param shader     Handle to a shader
 * @return           true if the shader was compiled, false otherwise
 */
bool shaderCompileStatus(GLuint shader)
{
   GLint compiled;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
   return compiled ? true : false;
}

/**
 * Retrieve a shader log
 *
 * @param shader     Handle to a shader
 * @return           The contents of the log
 */
std::string getShaderLog(GLuint shader)
{
   // Get the size of the log and allocate the required space
   GLint size;
   glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);

   // Allocate space for the log
   char* log = new char[size];
 
   // Get the shader log
   glGetShaderInfoLog(shader, size, NULL, log);

   // Convert it into a string (not efficient)
   std::string retval(log);

   // Free up space
   delete [] log;
   
   return retval;
}

/**
 * Check the link status of a program
 *
 * @param shader     Handle to a shader
 * @return           true if the shader was compiled, false otherwise
 */
bool programLinkStatus(GLuint program)
{
   GLint linked;
   glGetProgramiv(_program, GL_LINK_STATUS, &linked);
   return linked ? true : false;
}

/**
 * Retrieve a GLSL program log
 *
 * @param shader     Handle to a GLSL program
 * @return           The contents of the log
 */
std::string getProgramLog(GLuint program)
{
   // Get the size of the log and allocate the required space
   GLint size;
   glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &size);
   char* log = new char[size];

   // Get the program log
   glGetProgramInfoLog(program, size, NULL, log);

   // Convert it into a string (blah)
   std::string retval(log);

   // Clean up and return
   delete [] log;
   return retval;
}

/**
 * Create a shader program from a source string. The caller should
 * check the compile status
 *
 * @param source        The shader source
 * @param shaderType    The type of shader (GL_VERTEX_SHADER, etc)
 * @return              A handle to the shader program. 0 if an
 *                      error occured.
 */
GLuint createShader(const std::string& source, GLenum shaderType)
{
   GLuint shader = glCreateShader(shaderType);
   
   const GLchar* sourcePtr0 = source.c_str();
   const GLchar** sourcePtr = &sourcePtr0;
   
   // Set the source and attempt compilation
   glShaderSource(shader, 1, sourcePtr, NULL);
   glCompileShader(shader);
   
   return shader;
}

/**
 * Create a GLSL program object from vertex and fragment shader files.
 *
 * @param  vShaderFile   The vertex shader filename
 * @param  fShaderFile   The fragment shader filename
 * @return handle to the GLSL program
 */
GLuint createGLSLProgram(const std::string& vShaderFile, const std::string& fShaderFile)
{
#ifdef SHADER_IN_SOURCE
   std::string vertexSource(_vertexSource);
   std::string fragmentSource(_fragmentSource);
#else
   std::string vertexSource   = readTextFile(vShaderFile);
   std::string fragmentSource = readTextFile(fShaderFile);
#endif
   
   _program = glCreateProgram();
   
   // Create vertex shader
   GLuint vertexShader  = createShader(vertexSource, GL_VERTEX_SHADER);

   // Check for compile errors
   if(!shaderCompileStatus(vertexShader))
   {
      std::cerr << "Could not compile " << vShaderFile << std::endl;
      std::cerr << getShaderLog(vertexShader) << std::endl;
      terminate(EXIT_FAILURE);
   }

   // Create fragment shader
   GLuint fragmentShader = createShader(fragmentSource, GL_FRAGMENT_SHADER);
   
   // Check for compile errors
   if(!shaderCompileStatus(fragmentShader))
   {
      std::cerr << "Could not compile " << fShaderFile << std::endl;
      std::cerr << getShaderLog(fragmentShader) << std::endl;
      terminate(EXIT_FAILURE);
   }

   // Attach the shaders to the program
   glAttachShader(_program, vertexShader);
   glAttachShader(_program, fragmentShader);
   
   // Link the program
   glLinkProgram(_program);
   
   // Check for linker errors
   if(!programLinkStatus(_program))
   {
      std::cerr << "GLSL program failed to link:" << std::endl;
      std::cerr << getProgramLog(_program) << std::endl;
      terminate(EXIT_FAILURE);
   }

   return _program;
}

/**
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
void init(void)
{
   // Vertices of a unit cube centered at origin, sides aligned with axes
   vec4 points[8] = {
      vec4( -0.5, -0.5,  0.5, 1.0 ),
      vec4( -0.5,  0.5,  0.5, 1.0 ),
      vec4(  0.5,  0.5,  0.5, 1.0 ),
      vec4(  0.5, -0.5,  0.5, 1.0 ),
      vec4( -0.5, -0.5, -0.5, 1.0 ),
      vec4( -0.5,  0.5, -0.5, 1.0 ),
      vec4(  0.5,  0.5, -0.5, 1.0 ),
      vec4(  0.5, -0.5, -0.5, 1.0 )
   };
   
   // RGBA olors
   vec4 colors[8] = {
      vec4( 0.0, 0.0, 0.0, 1.0 ),  // black
      vec4( 1.0, 0.0, 0.0, 1.0 ),  // red
      vec4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
      vec4( 0.0, 1.0, 0.0, 1.0 ),  // green
      vec4( 0.0, 0.0, 1.0, 1.0 ),  // blue
      vec4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
      vec4( 1.0, 1.0, 1.0, 1.0 ),  // white
      vec4( 0.0, 1.0, 1.0, 1.0 )   // cyan
   };

   // Vertex index order
   GLuint indices[] = {
      1, 0, 3, 1, 3, 2, // Face 1, two triangles
      2, 3, 7, 2, 7, 6,
      3, 0, 4, 3, 4, 7,
      6, 5, 1, 6, 1, 2,
      4, 5, 6, 4, 6, 7,
      5, 4, 0, 5, 0, 1  // Face 6, two triangles
   };
   
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
   
   std::string vertexFile = std::string(SOURCE_DIR) + "/vertex.c";
   std::string fragFile   = std::string(SOURCE_DIR) + "/fragment.c";
   _program = createGLSLProgram(vertexFile, fragFile);

   // Get vertex and color attribute locations
   _vertexLocation = glGetAttribLocation(_program, "vertex");
   _colorLocation  = glGetAttribLocation(_program, "color");
      
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
   glBufferData(GL_ARRAY_BUFFER,          // Target buffer object
                8 * 4 * sizeof(GLfloat),  // Size in bytes of the buffer 
                (GLfloat*) points,        // Pointer to the data
                GL_STATIC_DRAW);          // Expected data usage pattern
   
   // Specify the location and data format of the array of generic vertex attributes
   glVertexAttribPointer(_vertexLocation, // Attribute location in the shader program
                         4,               // Number of components per attribute
                         GL_FLOAT,        // Data type of attribute
                         GL_FALSE,        // GL_TRUE: values are normalized or
                         // GL_FALSE: values are converted to fixed point
                         0,               // Stride
                         0);              // Offset into VBO for this data
   
   // Enable the generic vertex attribute array
   glEnableVertexAttribArray(_vertexLocation);
   
   // Set up color attributes
   glGenBuffers(1, &_colors);
   glBindBuffer(GL_ARRAY_BUFFER, _colors);
   glBufferData(GL_ARRAY_BUFFER,
                8 * 4 * sizeof(GLfloat),
                (GLfloat*) colors,
                GL_STATIC_DRAW);
   _colorLocation = glGetAttribLocation(_program, "color");
   glVertexAttribPointer(_colorLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(_colorLocation);

   // Create the index buffer
   glGenBuffers(1, &_indices);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);

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
   
   std::cout << "tracking: " << _tracking << std::endl;
   
   
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
   glm::mat4 model = _trackball->getTransform();
   
   // Create  model, view, projection matrix
   glm::mat4 mvp        = projection * view * model; // Remember, matrix multiplication is the other way around
   
   // Use the shader program that was loaded, compiled and linked
   glUseProgram(_program);
   
   // Set the MVP uniform
   glUniformMatrix4fv(_mvp, 1, GL_FALSE, &mvp[0][0]);

   glDrawElements(GL_TRIANGLES,     // Primitive to draw
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
      glfwSwapBuffers();
   }

   terminate(EXIT_SUCCESS);
}
