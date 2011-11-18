#ifndef _GLSLProgram_h
#define _GLSLProgram_h

#include <string>
#include <sstream>
#include <map>
#include "OpenGL.h"


namespace GL
{
   /**
    * Turn an OpenGL enumerated error into a descriptive
    * std::string
    *
    * @param error   The OpenGL error
    * @return        a descriptive error string
    */
   std::string errorString(GLenum error);

   /**
    * An OpenGL GLSL shader
    */
   class Shader
   {
   public:
      /**
       * Create a shader program from a file. The caller should check the
       * compile status
       *
       * @param filename      The name of the file with the shader source
       * @param shaderType    The type of shader (GL_VERTEX_SHADER, etc)
       */
      Shader(const std::string& filename, GLenum shaderType);
      
      /**
       * Destructor
       */
      ~Shader();
      
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
   
   
   /**
    * An OpenGL GLSL program
    */
   class Program
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
      Program(const std::string& vertexFile, const std::string& fragmentFile);

      /**
       * Create a GLSL program
       *
       * @param vertexFile
       *    The name of the file that contains vertex shader source
       * @param fragmentFile
       *    The name of the file that contains the fragment shader source
       * @param geometryFile
       *    The name of the file that contains the geometry shader source
       */
      Program(const std::string& vertexFile, const std::string& fragmentFile,
              const std::string& geometryFile);

      /**
       * Destructor
       */
      ~Program();

      /**
       * Map the names of uniforms to indices
       */
      void mapUniformNamesToIndices(void);
      
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
      
      /**
       * @return the number of shader objects attached to program.
       */
      const GLint getAttachedShaders(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ATTACHED_SHADERS, &params);
         return params;
      }
      
      /**
       * @return the number of active attribute variables for program.
       */
      const GLint getActiveAttributes(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTES, &params);
         return params;
      }
      
      /**
       * @return the length of the longest active attribute name for program, including
       *   the null termination character (i.e., the size of the character buffer required
       *   to store the longest attribute name). If no active attributes exist, 0 is 
       *   returned.
       */
      const GLint getActiveAttributeMaxLength(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &params);
         return params;
      }
      
      /**
       * @return the number of active uniform variables for program.
       */
      const GLint getActiveUniforms(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ACTIVE_UNIFORMS, &params);
         return params;
      }
      
      /**
       * Get the name of a uniform variable at the specified index
       *
       * @param index
       *   The index for the uniform
       *
       * @return The name of the uniform
       */
      const std::string getUniformName(GLuint index) const
      {
         static const GLsizei maxNameSize = 256;
         GLchar  glName[maxNameSize];
         GLsizei length;
         GLint   size;
         GLenum  type;
         glGetActiveUniform(_handle, index, maxNameSize, &length, &size, &type, glName);
         std::string name(glName);
         return name;
      }

      /**
       * Modifies the value of a uniform variable array
       *
       * @param name       Specifies the name of the uniform variable
       *
       * @param count      Specifies the number of matrices that are to be
       *                   modified. This should be 1 if the targeted uniform
       *                   variable is not an array of matrices, and 1 or
       *                   more if it is an array of matrices.
       *
       * @param transpose  Specifies whether to transpose the matrix as the
       *                   values are loaded into the uniform variable.
       *
       * @param value      Specifies a pointer to an array of count values that
       *                   will be used to update the specified uniform variable.
       */
      void setUniformMatrix4fv(const std::string& name, GLsizei count, GLboolean transpose,
                               const GLfloat* value)
      {
#ifdef ROBUST_UNIFORM_LOCATIONS
         glUniformMatrix4fv(getUniformLocation(name), count, transpose, value);
#else
         glUniformMatrix4fv(_uniform[name], count, transpose, value);
#endif
      }
      
      void setUniform4fv(const std::string& name, GLsizei count, const GLfloat *value)
      {
#ifdef ROBUST_UNIFORM_LOCATIONS
         glUniform4fv(getUniformLocation(name), count, value);
#else
         glUniform4fv(_uniform[name], count, value);
#endif
      }
      
      /**
       * @return the length of the longest active uniform variable name for program,
       *   including the null termination character (i.e., the size of the character buffer
       *   required to store the longest uniform variable name). If no active uniform 
       *   variables exist, 0 is returned.
       */
      const GLint getActiveUniformMaxLength(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &params);
         return params;
      }

   private:
      
      GLuint                        _handle;         //< OpenGL handle for a GLSL shader
      Shader*                       _vertexShader;   //< Pointer to the fragment shader
      Shader*                       _fragmentShader; //< Pointer to the vertex shader
      Shader*                       _geometryShader; //< Pointer to the geometry shader
      std::map<std::string, GLuint> _uniform;        //< Map of uniform names to GLuint indices
      std::map<std::string, GLuint> _attribute;      //< Map of attribute names to GLuint indices

   };
}
#endif
