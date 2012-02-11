#ifndef _shader_h
#define _shader_h

#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <vector>
#include "opengl.h"
#if 0
#include <mat4.h>
#include <vec.h>
#endif
namespace GL
{
   /**
    * Turn OpenGL errors into strings
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
       * Map the names of attributes to indices
       */
      void mapAttributeNamesToIndices(void);

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
       * Define an array of generic vertex attribute data.
       *
       * @param name
       *    Specifies the name of the vertex attribute.
       * @param size
       *    Specifies the number of components per generic vertex attribute.
       *    Must be 1, 2, 3, 4. Additionally, the symbolic constant GL_BGRA
       *    is accepted by glVertexAttribPointer. The initial value is 4.
       * @param type
       *    Specifies the data type of each component in the array. The symbolic
       *    constants GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, 
       *    GL_INT, and GL_UNSIGNED_INT are accepted by both functions.
       *    Additionally GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_INT_2_10_10_10_REV,
       *    and GL_UNSIGNED_INT_2_10_10_10_REV are accepted by glVertexAttribPointer. 
       *    The initial value is GL_FLOAT.
       * @param normalized
       *    For glVertexAttribPointer, specifies whether fixed-point data values
       *    should be normalized (GL_TRUE) or converted directly as fixed-point
       *    values (GL_FALSE) when they are accessed.
       * @param stride
       *    Specifies the byte offset between consecutive generic vertex attributes.
       *    If stride is 0, the generic vertex attributes are understood to be 
       *    tightly packed in the array. The initial value is 0.
       * @param pointer
       *    Specifies a offset of the first component of the first generic vertex 
       *    attribute in the array in the data store of the buffer currently bound
       *    to the GL_ARRAY_BUFFER target. The initial value is 0.
       */
      void setVertexAttribPointer(const std::string& name,
                                  GLint size,
                                  GLenum type,
                                  GLboolean normalized,
                                  GLsizei stride,
                                  const GLvoid * pointer)
      {
         glVertexAttribPointer(_attrib[name], size, type, normalized, stride, pointer);
      }
      
      /**
       * Enable or disable a generic vertex attribute array
       *
       * @param name
       *    Specifies the name of the vertex attribute
       */
      void enableVertexAttribArray(const std::string& name)
      {
         glEnableVertexAttribArray(_attrib[name]);
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
      
#ifndef OPENGL3
      /**
       * Change back to fixed function pipeline. Not available for OpenGL 3 programs
       */
      void release(void)
      {
         glUseProgram(0);
      }
#endif
      
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

      //{@ glUniform1i
      void setUniform(const std::string& name, const GLint v0) {
         glUniform1i(_uniform[name], v0);
      }
      

      void setUniform(const GLint id, const GLint v0) {
         glUniform1i(id, v0);
      }
      
      void setUniform1i(const std::string& name, const GLint v0)
      {
         glUniform1i(_uniform[name], v0);
      }
      
      void setUniform1i(const GLint id, const GLint v0) {
         glUniform1i(id, v0);
      }

      
      void setUniform(const std::string& name, const size_t v0) {
         glUniform1i(_uniform[name], v0);
      }
      
      void setUniform(const GLint id, const size_t v0) {
         glUniform1i(id, v0);
      }
      

      
      //@}
      
      //{@ glUniform1f
      void setUniform(const std::string& name, const GLfloat v0)
      {
         glUniform1f(_uniform[name], v0);
      }

      void setUniform(const GLint id, GLfloat v0)
      {
         glUniform1f(id, v0);
      }
      
      void setUniform1f(const std::string& name, const GLfloat v0)
      {
         glUniform1f(_uniform[name], v0);
      }
      
      void setUniform1f(const GLint id, GLfloat v0)
      {
         glUniform1f(id, v0);
      }
      //@}
      
#if 0
      void setUniform(const std::string& name, GLfloat v0, GLfloat v1)
      {
         glUniform2f(_uniform[name], v0, v1);
         
      }

      void setUniform(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
      {
         glUniform3f(_uniform[name], v0, v1, v2);
         
      }

      void setUniform(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
      {
         glUniform4f(_uniform[name], v0, v1, v2, v3);
      }
      

      
      void setUniform(const std::string& name, GLint v0, GLint v1)
      {
         glUniform2i(_uniform[name], v0, v1);
      }
      
      void setUniform(const std::string& name, GLint v0, GLint v1, GLint v2)
      {
         glUniform3i(_uniform[name], v0, v1, v2);
      }
      
      void setUniform(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
      {
         glUniform4i(_uniform[name], v0, v1, v2, v3);
      }
      
      void setUniform1ui(const std::string& name, GLuint v0)
      {
         glUniform1ui(_uniform[name], v0);
         
      }
      
      void setUniform(const std::string& name, GLuint v0, GLuint v1)
      {
         glUniform2ui(_uniform[name], v0, v1);
         
      }
      
      void setUniform(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
      {
         glUniform3ui(_uniform[name], v0, v1, v2);
         
      }
      
      void setUniform(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
      {
         glUniform4ui(_uniform[name], v0, v1, v2, v3);
      }

      void setUniform1uiv(const std::string& name, GLsizei length, GLuint* data) 
      {
         glUniform1uiv(_uniform[name], length, data);
      }
      
      void setUniform1uiv(const GLint id, GLsizei length, GLuint* data) 
      {
         glUniform1uiv(id, length, data);
      }
#endif

      void setUniform(const std::string& name, const std::vector<int>& data) {
         glUniform1iv(_uniform[name], data.size(), &data[0]);
      }
      
      /**
       * Modifies the value of a uniform variable array
       *
       * @param name
       * Specifies the name of the uniform var
       *
       * @param count
       * Specifies the number of matrices that are to be modified. This
       * should be 1 if the targeted uniform variable is not an array of
       * matrices, and 1 or more if it is an array of matrices.
       *
       * @param transpose
       * Specifies whether to transpose the matrix as the values are loaded
       * into the uniform variable.
       *
       * @param value
       * Specifies a pointer to an array of count values that will be used
       * to update the specified uniform variable.
       */
      void setUniformMatrix4(const std::string& name, GLsizei count, GLboolean transpose,
                               const GLfloat* value)
      {
#ifdef ROBUST_UNIFORM_LOCATIONS
         glUniformMatrix4fv(getUniformLocation(name), count, transpose, value);
#else
         glUniformMatrix4fv(_uniform[name], count, transpose, value);
#endif
      }
      
      void setUniform4(const std::string& name, GLsizei count, const GLfloat *value)
      {
#ifdef ROBUST_UNIFORM_LOCATIONS
         glUniform4fv(getUniformLocation(name), count, value);
#else
         glUniform4fv(_uniform[name], count, value);
#endif
      }
#if 0
      void setUniform(const std::string& name, const gutz::mat4f& mat)
      {
         glUniformMatrix4fv(_uniform[name], 1, GL_FALSE, mat.m);
      }
#endif 
   private:
      
      GLuint                        _handle;         //< OpenGL handle for a GLSL shader
      Shader*                       _vertexShader;   //< Pointer to the fragment shader
      Shader*                       _fragmentShader; //< Pointer to the vertex shader
      Shader*                       _geometryShader; //< Pointer to the geometry shader
      std::map<std::string, GLuint> _uniform;        //< Map of uniform names to GLuint indices
      std::map<std::string, GLuint> _attrib;         //< Map of attribute names to GLuint indices

   };
}
#endif
