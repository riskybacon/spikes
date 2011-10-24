#ifndef _GLSLProgram_h
#define _GLSLProgram_h

#include <string>

#define OPENGL3

#if defined(__APPLE_CC__)
  #ifdef OPENGL3
    #include <OpenGL/gl3.h>
  #else
    #include <OpenGL/gl.h>
  #endif
#else
  #include <GL/gl.h>
#endif

class GLSLShader
{
public:
   /**
    * Create a shader program from a file. The caller should check the
    * compile status
    *
    * @param filename      The name of the file with the shader source
    * @param shaderType    The type of shader (GL_VERTEX_SHADER, etc)
    */
   GLSLShader(const std::string& filename, GLenum shaderType);
   
   /**
    * Check the compile status of a shader
    *
    * @param shader     Handle to a shader
    * @return           true if the shader was compiled, false otherwise
    */
   bool getCompileStatus() const;
   
   /**
    * Retrieve a shader log
    *
    * @return           The contents of the log
    */
   std::string getLog(void) const;
   
   /**
    * @return OpenGL handle for the shader
    */
   GLuint getHandle(void) const
   {
      return _handle;
   }
private:
   
   GLuint _handle; //< OpenGL handle for a GLSL shader
};


class GLSLProgram
{
public:
   /**
    * Create a GLSL program
    *
    * @param vertexFile
    *    The name of the file that contains vertex shader source
    * @param fragmentFile
    *    The name of the file that contains the fragment shader source
    */
   GLSLProgram(const std::string& vertexFile, const std::string& fragmentFile);
   
   /**
    * Check the link status of the program
    *
    * @param shader     Handle to a shader
    * @return           true if the shader was compiled, false otherwise
    */
   bool getLinkStatus() const;
   
   /**
    * Retrieve the program log
    *
    * @return           The contents of the log
    */
   std::string getLog(void) const;
   
   /**
    * Get the location of an program attribute
    *
    * @param name
    *    The name of the attribute
    * @return the location of the program attribute
    */
   GLuint getAttribLocation(const std::string& name) const
   {
      return glGetAttribLocation(_handle, name.c_str());
   }
   
   /**
    * Get the location of an program uniform variable
    *
    * @param name
    *    The name of the uniform variable
    * @return the location of the uniform variable
    */
   GLuint getUniformLocation(const std::string& name) const
   {
      return glGetUniformLocation(_handle, name.c_str());
   }
   
   /**
    * @return OpenGL handle for the shader
    */
   GLuint getHandle(void) const
   {
      return _handle;
   }
   
   /**
    * Bind this program to the current OpenGL state
    */
   void bind(void)
   {
      glUseProgram(_handle);
   }

private:
   
   GLuint      _handle;         //< OpenGL handle for a GLSL shader
   GLSLShader* _vertexShader;   //< Pointer to the fragment shader
   GLSLShader* _fragmentShader; //< Pointer to the vertex shader
};

#endif
