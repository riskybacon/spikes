#include <iostream>
#include <fstream>
#include "glslprogram.h"

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

GLSLShader::GLSLShader(const std::string& filename, GLenum shaderType)
{
   std::string source = readTextFile(filename);
   const GLchar* sourcePtr0 = source.c_str();
   const GLchar** sourcePtr = &sourcePtr0;
   
   // Set the source and attempt compilation
   _handle = glCreateShader(shaderType);
   
   if(_handle == 0)
   {
      std::cerr << "Could not create a shader program" << std::endl;
   }
   glShaderSource(_handle, 1, sourcePtr, NULL);
   glCompileShader(_handle);
   
   if(!getCompileStatus())
   {
      std::cerr << "Failed to compile shader file: " << filename << std::endl;
      std::cerr << getLog() << std::endl;
   }
}

bool GLSLShader::getCompileStatus(void) const
{
   GLint compiled;
   glGetShaderiv(_handle, GL_COMPILE_STATUS, &compiled);
   return compiled ? true : false;
}

std::string GLSLShader::getLog(void) const
{
   // Get the size of the log and allocate the required space
   GLint size;
   glGetShaderiv(_handle, GL_INFO_LOG_LENGTH, &size);
   
   // Allocate space for the log
   char* log = new char[size];
   
   // Get the shader log
   glGetShaderInfoLog(_handle, size, NULL, log);
   
   // Convert it into a string (not efficient)
   std::string retval(log);
   
   // Free up space
   delete [] log;
   
   return retval;
}

GLSLProgram::GLSLProgram(const std::string& vShaderFile, const std::string& fShaderFile)
{
   _handle = glCreateProgram();
   
   _vertexShader   = new GLSLShader(vShaderFile, GL_VERTEX_SHADER);
   _fragmentShader = new GLSLShader(fShaderFile, GL_FRAGMENT_SHADER);
   glAttachShader(_handle, _vertexShader->getHandle());
   glAttachShader(_handle, _fragmentShader->getHandle());
   
   
   // Link the program
   glLinkProgram(_handle);
   
   // Check for linker errors
   if(!getLinkStatus())
   {
      std::cerr << "GLSL program failed to link:" << std::endl;
      std::cerr << getLog() << std::endl;
   }
}

/**
 */
bool GLSLProgram::getLinkStatus(void) const
{
   GLint linked;
   glGetProgramiv(_handle, GL_LINK_STATUS, &linked);
   return linked ? true : false;
}

/**
 */
std::string GLSLProgram::getLog(void) const
{
   // Get the size of the log and allocate the required space
   GLint size;
   glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &size);
   char* log = new char[size];
   
   // Get the program log
   glGetProgramInfoLog(_handle, size, NULL, log);
   
   // Convert it into a string (blah)
   std::string retval(log);
   
   // Clean up and return
   delete [] log;
   return retval;
}
