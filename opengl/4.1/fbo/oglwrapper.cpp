#include <iostream>
#include <fstream>
#include "oglwrapper.h"

namespace GL
{
   std::string errorString(GLenum error)
   {
      std::string errorString;
      switch(error)
      {
            
         case GL_NO_ERROR:
            errorString = "No error has been recorded.";
            break;
            
         case GL_INVALID_ENUM:
            errorString = "An unacceptable value was specified "
            "for an enumerated argument. The offending "
            "command has been ignored, and has no other "
            "side effect than to set the error flag.";
            break;
            
         case GL_INVALID_VALUE:
            errorString = "A numeric argument is out of range. "
            "The offending command has been ignored, and "
            "has no other side effect than to set the error "
            "flag.";
            break;
            
         case GL_INVALID_OPERATION:
            errorString = "The specified operation is not "
            "allowed in the current state. The offending "
            "command has ignored, and has no other side "
            "effect than to set the error flag. ";
            break;
            
         case GL_OUT_OF_MEMORY:
            errorString = "There is not enough memory left to "
            "execute the command. The state of OpenGL is now "
            "undefined";
            break;
            
#ifndef OPENGL3
            // The following errors cannot occur in OpenGL 3.2
            // or higher, due to the removal of stacks and tables
         case GL_STACK_OVERFLOW:
            errorString = "The command would cause a stack "
            "overflow. The offending command has been "
            "ignored, and has no other side effect than to "
            "set the error flag.";
            break;
            
         case GL_STACK_UNDERFLOW:
            errorString = "This  command  would cause a stack "
            "underflow. The offending command has been "
            "ignored, and has no other side effect than to "
            "set the error flag. ";
            break;
            
         case GL_TABLE_TOO_LARGE:
            errorString = "The specified table exceeds the "
            "implementation's  maximum  supported table size. "
            "The offending command was ignored, and has no "
            "other side effect than to set the error flag.";
            break;
            
#endif
            
         default:
            errorString = "An undefined OpenGL error has occurred.";
            break;
      }
      
      return errorString;
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
   
   Shader::Shader(const std::string& filename, GLenum shaderType)
   : _handle   (0)
   {
      std::string source = readTextFile(filename);
      const GLchar* sourcePtr0 = source.c_str();
      const GLchar** sourcePtr = &sourcePtr0;
      
      // Set the source and attempt compilation
      _handle = glCreateShader(shaderType);
      GL_ERR_CHECK();

      glShaderSource(_handle, 1, sourcePtr, NULL);
      GL_ERR_CHECK();

      glCompileShader(_handle);
      GL_ERR_CHECK();

      if(!getCompileStatus())
      {
         std::stringstream err;
         err << "Failed to compile shader file: " << filename << std::endl;
         err << getLog() << std::endl;
         throw GL::Exception(err.str());
      }
   }
   
   Shader::~Shader()
   {
      if(_handle > 0)
      {
         glDeleteShader(_handle);
      }
   }
   
   bool Shader::getCompileStatus(void) const
   {
      GLint compiled;
      glGetShaderiv(_handle, GL_COMPILE_STATUS, &compiled);
      GL_ERR_CHECK();
      return compiled ? true : false;
   }
   
   std::string Shader::getLog(void) const
   {
      // Get the size of the log and allocate the required space
      GLint size;
      glGetShaderiv(_handle, GL_INFO_LOG_LENGTH, &size);
      GL_ERR_CHECK();
      
      // Allocate space for the log
      char* log = new char[size];
      
      // Get the shader log
      glGetShaderInfoLog(_handle, size, NULL, log);
      GL_ERR_CHECK();
      
      // Convert it into a string (not efficient)
      std::string retval(log);
      
      // Free up space
      delete [] log;
      
      return retval;
   }
   
   Program::Program(const std::string& vShaderFile, const std::string& fShaderFile)
   {
      _handle = glCreateProgram();
      GL_ERR_CHECK();
      
      _vertexShader   = new Shader(vShaderFile, GL_VERTEX_SHADER);
      _fragmentShader = new Shader(fShaderFile, GL_FRAGMENT_SHADER);

      glAttachShader(_handle, _vertexShader->getHandle());
      GL_ERR_CHECK();

      glAttachShader(_handle, _fragmentShader->getHandle());
      GL_ERR_CHECK();
      
      
      // Link the program
      glLinkProgram(_handle);
      GL_ERR_CHECK();
      
      // Check for linker errors
      if(!getLinkStatus())
      {
         std::stringstream err;
         err << "GLSL program failed to link:" << std::endl;
         err << getLog() << std::endl;
         throw GL::Exception(err.str());
      }
   }
   
   Program::~Program()
   {
      if(_handle > 0)
      {
         delete _vertexShader;
         delete _fragmentShader;
         glDeleteProgram(_handle);
      }
   }
   /**
    */
   bool Program::getLinkStatus(void) const
   {
      GLint linked;
      glGetProgramiv(_handle, GL_LINK_STATUS, &linked);
      GL_ERR_CHECK();
      return linked ? true : false;
   }
   
   /**
    */
   std::string Program::getLog(void) const
   {
      // Get the size of the log and allocate the required space
      GLint size;
      glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &size);
      GL_ERR_CHECK();
      char* log = new char[size];
      
      // Get the program log
      glGetProgramInfoLog(_handle, size, NULL, log);
      GL_ERR_CHECK();
      
      // Convert it into a string (blah)
      std::string retval(log);
      
      // Clean up and return
      delete [] log;
      return retval;
   }
   
   ///////////////////////////////////////////////////////////////////////////////////////
   /// Build a mapping of uniform names to uniform indices
   ///////////////////////////////////////////////////////////////////////////////////////
   void Program::mapUniformNamesToIndices(void)
   {
      for(int i = 0; i < getActiveUniforms(); ++i)
      {
         _uniform[getUniformName(i)] = i;
      }
      
#if 1
      std::map<std::string, GLuint>::const_iterator itr;
      for(itr = _uniform.begin(); itr != _uniform.end(); ++itr)
      {
         std::cout << "(name, index): (" << itr->first << "," << itr->second << ")" << std::endl;
      }
#endif
      
   }

}