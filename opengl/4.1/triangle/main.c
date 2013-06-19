// main.cpp
//
// Jeff Bowles <jbowles@riskybacon.com>

#include <stdio.h>
#include <stdlib.h>

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

#include <GLFW/glfw3.h>

// Set the OpenGL core context version
#define GL_MAJOR 3
#define GL_MINOR 2

// Global variables have an underscore prefix.
GLuint _program;        //< Shader program handle
GLuint _vao;            //< Vertex array object for the vertices
GLuint _vertices;       //< Vertex buffer object for the vertices
GLint  _vertexLocation; //< Location of the vertex attribute in the shader program

// Vertex shader source
// This shader passes the vertex position through unchanged
const char* _vertexSource = 
"#version 150\n"
"\n"
"in vec4 vertex;\n"
"\n"
"void main(void)\n"
"{\n"
"	gl_Position = vertex;\n"
"}\n";

// Fragment shader source
// This shader colors all fragments the same
const char* _fragmentSource =
"#version 150\n"
"\n"
"out vec4 fragColor;\n"
"\n"
"void main(void)\n"
"{\n"
"	fragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
"}\n";

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
 * Credits: Ed Angel, Interactive Computer Graphics, 5th Edition
 * jbowles added fopen_s portion for better cross-platform support
 *
 * @param filename	The name of the file
 * @return			   A string that contains the contents of the file
 */
char* readShaderSource(const char* shaderFile)
{
   FILE* fp;
   
   char* buf;
   long size;

#ifdef WIN32
   errno_t err;
   if((err = fopen_s(&fp, shaderFile, "r")))
#else
   if((fp = fopen(shaderFile, "r")))
#endif
   {
      fprintf(stderr, "Cannot open %s\n", shaderFile);
      terminate(EXIT_FAILURE);
   }
      
   fseek(fp, 0L, SEEK_END);
   size = ftell(fp);
   fseek(fp, 0L, SEEK_SET);
   buf = (char*) malloc((size+1) * sizeof(char));
   fread(buf, 1, size, fp);
   buf[size] = '\0';
   fclose(fp);
   return buf;
}

/**
 * Check the compile status of a shader
 *
 * @param shader     Handle to a shader
 * @return           1 if the shader was compiled, 0 otherwise
 */
int shaderCompileStatus(GLuint shader)
{
   GLint compiled;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
   return compiled ? 1 : 0;
}

/**
 * Retrieve a shader log
 *
 * @param shader     Handle to a shader
 * @return           The contents of the log
 */
char* getShaderLog(GLuint shader)
{
   // Get the size of the log and allocate the required space
   GLint size;
   char* log;
   glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);

   // Allocate space for the log
   log = (char*) malloc(size * sizeof(char));
 
   // Get the shader log
   glGetShaderInfoLog(shader, size, NULL, log);

   return log;
}

/**
 * Check the link status of a program
 *
 * @param shader     Handle to a shader
 * @return           1 if the shader was compiled, 0 otherwise
 */
int programLinkStatus(GLuint program)
{
   GLint linked;
   glGetProgramiv(_program, GL_LINK_STATUS, &linked);
   return linked ? 1 : 0;
}

/**
 * Retrieve a GLSL program log
 *
 * @param shader     Handle to a GLSL program
 * @return           The contents of the log
 */
char* getProgramLog(GLuint program)
{
   // Get the size of the log and allocate the required space
   GLint size;
   char* log;
   glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &size);
   
   // Allocate space for the log
   log = (char*) malloc(size * sizeof(char));

   // Get the program log
   glGetProgramInfoLog(program, size, NULL, log);

   return log;
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
GLuint createShader(const char* source, GLenum shaderType)
{
   GLuint shader = glCreateShader(shaderType);
   
   const GLchar* sourcePtr0 = source;
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
GLuint createGLSLProgram(const char* vertexSource, const char* fragmentSource)
{
	GLuint vertexShader;
	GLuint fragmentShader;
	char* log;
   _program = glCreateProgram();
   
   // Create vertex shader
   vertexShader  = createShader(vertexSource, GL_VERTEX_SHADER);

   // Check for compile errors
   if(!shaderCompileStatus(vertexShader))
   {
      log = getShaderLog(vertexShader);
      fprintf(stderr, "Could not compile vertex shader:\n%s\n", log);
      free(log);
      terminate(EXIT_FAILURE);
   }

   // Create fragment shader
   fragmentShader = createShader(fragmentSource, GL_FRAGMENT_SHADER);
   
   // Check for compile errors
   if(!shaderCompileStatus(fragmentShader))
   {
      log = getShaderLog(fragmentShader);
      fprintf(stderr, "Could not compile fragment shader:\n%s\n", log);
      free(log);
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
      log = getProgramLog(_program);
      fprintf(stderr, "GLSL program filed to link:\n%s\n", log);
      free(log);
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
   // Points of a triangle.
   GLfloat points[] = {-1.0f, -0.75f, 0.0f, 1.0f,
                        0.0f,  0.75f, 0.0f, 1.0f,
                        1.0f, -0.75f, 0.0f, 1.0f };
  
#ifndef __APPLE__
   // GLEW has trouble supporting the core profile
   glewExperimental = GL_TRUE;
   glewInit();
   if(!GLEW_ARB_vertex_array_object)
   {
      fprintf(stderr, "ARB_vertex_array_object not available.\n");
      terminate(EXIT_FAILURE);
   }
#endif

   _program = createGLSLProgram(_vertexSource, _fragmentSource);
   // Get the location of the "vertex" attribute in the shader program
   _vertexLocation = glGetAttribLocation(_program, "vertex");

   // Generate a single handle for a vertex array object
   glGenVertexArrays(1, &_vao);

   // Generate a handle for a buffer object
   glGenBuffers(1, &_vertices);

   // Bind the vertex array object
   glBindVertexArray(_vao);

   // Make that vbo the current array buffer. Subsequent array buffer operations
   // will affect this vbo.
   glBindBuffer(GL_ARRAY_BUFFER, _vertices);
   
   // Set the data for the vbo.
   glBufferData(GL_ARRAY_BUFFER,          // Target buffer object
                3 * 4 * sizeof(GLfloat),  // Size in bytes of the buffer 
                (GLfloat*) points,        // Pointer to the data
                GL_STATIC_DRAW);          // Expected data usage pattern

   // Enable the vertex attribute array for the currently bound vertex array object
   glEnableVertexAttribArray(_vertexLocation);
   
   // Specify the location and data format of the array of generic vertex attributes
   glVertexAttribPointer(_vertexLocation, // Attribute location in the shader program
                         4,               // Number of components per attribute
                         GL_FLOAT,        // Data type of attribute
                         GL_FALSE,        // GL_TRUE: values are normalized or
                         // GL_FALSE: values are converted to fixed point
                         0,               // Stride
                         0);              // Offset into VBO for this data

   // Set the clear color
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   
   // Set the depth clearing value
   glClearDepth(1.0f);
   
   glBindVertexArray(0);
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
   // Clear the color and depth buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Use the shader program that was loaded, compiled and linked
   glUseProgram(_program);
   
   // Bind the previously defined vertex array object
   glBindVertexArray(_vao);

   // Draw the triangle.
   glDrawArrays(GL_TRIANGLES, 0, 3);
   
   return GL_TRUE;
}

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
   int width = 1024; // Initial window width
   int height = 768; // Initial window height

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
   
   glfwSetKeyCallback(window, keypress);
   glfwSetWindowSizeCallback(window, resize);
   glfwSetWindowCloseCallback(window, close);
   
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