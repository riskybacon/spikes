//
// OpenGL 3.2 Texture Mapping example, imaging loading handled by FreeImage
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// Include FreeImage
#include <FreeImage.h>

using namespace glm;
using std::vector;

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
mat4         _scale;           //< Scale needed to display the image using proper aspect ratio
float        _zoom;

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
 * Load a texture map from a file.
 * This function makes some assumptions and should work for most files. To
 * make it more generic and handle a greater range of file types would 
 * require some work and some understanding of FreeImage.
 * This should get you started
 */
void loadTexture(const std::string& filename)
{
   std::map<FREE_IMAGE_FORMAT, std::string> fiFormatString;
   
   fiFormatString[FIF_UNKNOWN] = std::string("Unknown");
   fiFormatString[FIF_BMP]     = std::string("BMP");
   fiFormatString[FIF_ICO]     = std::string("ICO");
   fiFormatString[FIF_JPEG]    = std::string("JPEG");
   fiFormatString[FIF_JNG]     = std::string("JNG");
   fiFormatString[FIF_KOALA]   = std::string("Koala");
   fiFormatString[FIF_LBM]     = std::string("LBM");
   fiFormatString[FIF_IFF]     = std::string("IFF");
   fiFormatString[FIF_MNG]     = std::string("MNG");
   fiFormatString[FIF_PBM]     = std::string("PBM");
   fiFormatString[FIF_PBMRAW]  = std::string("PBMRAW");
   fiFormatString[FIF_PCD]     = std::string("PCD");
   fiFormatString[FIF_PCX]     = std::string("PCX");
   fiFormatString[FIF_PGM]     = std::string("PGM");
   fiFormatString[FIF_PGMRAW]  = std::string("PGMRAW");
   fiFormatString[FIF_PNG]     = std::string("PNG");
   fiFormatString[FIF_PPM]     = std::string("PPM");
   fiFormatString[FIF_PPMRAW]  = std::string("PPMRAW");
   fiFormatString[FIF_RAS]     = std::string("RAS");
   fiFormatString[FIF_TARGA]   = std::string("TARGA");
   fiFormatString[FIF_TIFF]    = std::string("TIFF");
   fiFormatString[FIF_WBMP]    = std::string("WBMP");
   fiFormatString[FIF_PSD]     = std::string("PSD");
   fiFormatString[FIF_CUT]     = std::string("CUT");
   fiFormatString[FIF_XBM]     = std::string("XBM");
   fiFormatString[FIF_XPM]     = std::string("XPM");
   fiFormatString[FIF_DDS]     = std::string("DDS");
   fiFormatString[FIF_GIF]     = std::string("GIF");
   fiFormatString[FIF_HDR]     = std::string("HDR");
   fiFormatString[FIF_FAXG3]   = std::string("FAXG3");
   fiFormatString[FIF_SGI]     = std::string("SGI");
   fiFormatString[FIF_EXR]     = std::string("EXR");
   fiFormatString[FIF_J2K]     = std::string("J2K");
   fiFormatString[FIF_JP2]     = std::string("J2P");
   fiFormatString[FIF_PFM]     = std::string("PFM");
   fiFormatString[FIF_PICT]    = std::string("PICT");
   fiFormatString[FIF_RAW]     = std::string("RAW");

   
   std::stringstream err;
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN; //< Image format
	FIBITMAP*         bitmap = NULL;        //< Pointer to image
	BYTE*             imageData;            //< Image data

   
   std::cout << "Loading texture file " << filename << std::endl;
   
   err << "Error processing " << filename << ": ";
   
   // Determine the format of this file (PNG, JPEG, etc)
	format = FreeImage_GetFileType(filename.c_str(), 0);

   // If unknown, guess the format from the file extension
	if(format == FIF_UNKNOWN)
   {
		format = FreeImage_GetFIFFromFilename(filename.c_str());
   }
   
	// Still unknown, stop processing
	if(format == FIF_UNKNOWN)
   {
      err << "could not determine image file format";
      throw std::runtime_error(err.str());
   }

   std::cout << "Format: " << fiFormatString[format] << std::endl;

   // Check to see if this file can be read by FreeImage
	if(FreeImage_FIFSupportsReading(format))
   {
      bitmap = FreeImage_Load(format, filename.c_str());
      
      if(bitmap == NULL)
      {
         err << "unable to load file";
         throw std::runtime_error(err.str());
      }
   }
   else
   {
      err << "format " << fiFormatString[format] << " not supported by this build of FreeImage";
      throw std::runtime_error(err.str());
   }

   FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(bitmap);
   GLenum texFormat;
   GLint  internalFormat;
   
   switch(colorType)
   {
      case FIC_RGB:
         texFormat = GL_RGB;
         internalFormat = GL_BGR;
         break;
         
      case FIC_RGBALPHA:
         texFormat = GL_RGBA;
         internalFormat = GL_BGRA;
         break;

      case FIC_MINISWHITE:
         err << "FIC_MINISWHITE FREE_IMAGE_COLOR_TYPE is not supported";
         throw std::runtime_error(err.str());
         break;

      case FIC_MINISBLACK:
         err << "FIC_MINISBLACK FREE_IMAGE_COLOR_TYPE is not supported";
         throw std::runtime_error(err.str());
         texFormat = GL_RGB;
         internalFormat = GL_RGB;
         break;
         
      case FIC_PALETTE:
         err << "FIC_PALETTE FREE_IMAGE_COLOR_TYPE is not supported";
         throw std::runtime_error(err.str());
         break;
         
      case FIC_CMYK:
         err << "FIC_CMYK FREE_IMAGE_COLOR_TYPE is not supported";
         throw std::runtime_error(err.str());
         break;

      default:
         err << "Unkonw FREE_IMAGE_COLOR_TYPE returned by FreeImage_GetColorType";
         throw std::runtime_error(err.str());
         break;
   }

   FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(bitmap);
   GLenum dataType;
   
   switch(imageType)
   {
      case FIT_UNKNOWN:
         err << "FIT_UNKNOWN returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;
         
      case FIT_BITMAP:
         // This is a big assumption, but should work for most images.
         dataType = GL_UNSIGNED_BYTE;
         // Ideally, the number of bits per color channel would be determined and an appropriate
         // data type, like GL_FLOAT, would be set. It should be possible to determin this from
         // the following function calls and make conversions as necessary:
#if 0
         std::cout << "FreeImage_GetColorsUsed: " << FreeImage_GetColorsUsed(bitmap) << std::endl;
         std::cout << "FreeImage_GetBPP:        " << FreeImage_GetBPP(bitmap)        << std::endl;
         std::cout << "FreeImage_GetRedMask:    " << FreeImage_GetRedMask(bitmap)    << std::endl;
         std::cout << "FreeImage_GetGreenMask:  " << FreeImage_GetGreenMask(bitmap)  << std::endl;
         std::cout << "FreeImage_GetBlueMask:   " << FreeImage_GetBlueMask(bitmap)   << std::endl;
#endif
         break;

      case FIT_UINT16:
         err << "FIT_UINT16 returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;
         
      case FIT_INT16:
         err << "FIT_INT16 returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;

      case FIT_UINT32:
         dataType = GL_UNSIGNED_INT;
         break;
         
      case FIT_INT32:
         dataType = GL_INT;
         break;
         
      case FIT_FLOAT:
         dataType = GL_FLOAT;
         break;

      case FIT_DOUBLE:
         err << "FIT_DOUBLE returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;

      case FIT_COMPLEX:
         err << "FIT_COMPLEX returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;

      case FIT_RGB16:
         err << "FIT_RGBA16 (48-bit RGB integer image type) returned by FreeImage_GetImageType. This is unsupported";
         err << "FIT_ returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;

      case FIT_RGBA16:
         err << "FIT_RGBA16 (64-bit RGBA integer image type) returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;

      case FIT_RGBF:
         err << "FIT_RGBF (96-bit RGB float image type) returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;

      case FIT_RGBAF:
         err << "FIT_RGBAF (128-bit RGBA float image type) returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;
         
      default:
         err << "Unknown value returned by FreeImage_GetImageType. This is unsupported";
         throw std::runtime_error(err.str());
         break;

   }
   
   // retrieve the image data
	imageData = FreeImage_GetBits(bitmap);
   
   if(imageData == NULL)
   {
      err << "data pointer is NULL";
      throw std::runtime_error(err.str());
   }
	//get the image width and height
   _texWidth = FreeImage_GetWidth(bitmap);
   
   if(_texWidth <= 0)
   {
      err << "width of image is too small: " << _texWidth;
      throw std::runtime_error(err.str());
   }
   
	_texHeight = FreeImage_GetHeight(bitmap);

   if(_texHeight <= 0)
   {
      err << "height of image is too small: " << _texHeight;
      throw std::runtime_error(err.str());
   }
   
   try {
      // Set up the texture
      glGenTextures(1, &_texture);
      glBindTexture(GL_TEXTURE_2D, _texture);
      glTexImage2D(GL_TEXTURE_2D, 0, texFormat, _texWidth, _texHeight, 0, internalFormat, dataType, imageData);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glGenerateMipmap(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      GL_ERR_CHECK();
   }
   catch (std::runtime_error err)
   {
      logException(err);
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
      GLint attribLoc;
      
      const std::string textureFile = std::string(SOURCE_DIR) + "/automati.ttf_sdf.png";
      //const std::string textureFile = std::string(SOURCE_DIR) + "/cactus.ppm";

      initGLEW();
      loadTexture(textureFile);
      
      _scale = glm::scale(mat4(), vec3(1.0f, float(_texHeight) / float(_texWidth), 1.0f));
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
 * Scrolling callback
 */
void scroll(GLFWwindow*, double x, double y)
{
   _zoom += y;
   
   if(_zoom > 3)
   {
      _zoom = 3.0f;
   }
   
   if(_zoom < -100.0f)
   {
      _zoom = -100.0f;
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
      
      glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f + _zoom));
      
      
      // Model matrix 
      glm::mat4 model = _scale * glm::mat4_cast(_objRot);
      
      // Create  model, view, projection matrix
      glm::mat4 mvp   = projection * view * translate * model;
      
      // Use the shader program that was loaded, compiled and linked
      _program->bind();
      GL_ERR_CHECK();
      
      // Set the MVP uniform
      _program->setUniform("mvp", mvp);
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
   _sensitivity = float(M_PI) / 360.0f;
   _zoom = 0;
   
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
   window = glfwCreateWindow(1024, 768, "Signed Distance Field", NULL, NULL);
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
   glfwSetScrollCallback(window, scroll);
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