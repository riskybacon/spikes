#ifndef _GLSLProgram_h
#define _GLSLProgram_h

#include <string>
#include <sstream>

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

#ifdef __GNUC__
// There is a bug in version 4.4.5 of GCC on Ubuntu which causes GCC to segfault
// when __PRETTY_FUNCTION__ is used within certain templated functions. 
#  if !(__GNUC__ == 4 && __GNUC_MINOR__ == 4 && __GNUC_PATCHLEVEL__ == 5)
#    define GL_FUNCTION_NAME __PRETTY_FUNCTION__
#  else
#    define GL_FUNCTION_NAME "unknown function"
#  endif
#elif _MSC_VER
#define GL_FUNCTION_NAME __FUNCSIG__
#else
#define GL_FUNCTION_NAME "unknown function"
#endif


extern "C"
{
   /// This function does nothing. Called when GL_ASSERT fails and is about to 
   /// throw an exception. Put your breakpoint here.
   inline void assert_breakpoint() {}
}


#ifdef _DEBUG
#define GL_ASSERT(_exp, _message) \
{ \
   if(!(_exp)) \
   { \
      assert_breakpoint(); \
      std::ostringstream glassert__out; \
      glassert__out << "Error in file " << __FILE__ << ":" << __LINE__ << "\n";  \
      glassert__out << GL_FUNCTION_NAME << ".\n\n";                                     \
      glassert__out << "Failed expression: " << #_exp << ".\n";                           \
      glassert__out << std::boolalpha << _message << "\n";                                \
      throw GL::Exception(glassert__out.str());                                         \
   } \
}                                                                  

/// Throw an exception if there are any OpenGL errors
#define GL_ERR_CHECK() \
{ \
   GLuint errnum; \
   std::ostringstream gl__out; \
   int n = 0; \
   while((errnum = glGetError()) && n < 10) \
   { \
      if(n == 0) \
      { \
         gl__out << "Error in file " << __FILE__ << ":" << __LINE__ << "\n"; \
         gl__out << GL_FUNCTION_NAME << ".\n\n"; \
      } \
      ++n; \
      gl__out << GL::errorString(errnum) << "\n"; \
   } \
   if(n > 0) \
   { \
      assert_breakpoint(); \
      throw GL::Exception(gl__out.str()); \
   } \
}

#else
#define GL_ERR_CHECK()
#define GL_ASSERT();
#endif

//gl__out << reinterpret_cast<const char *>(GL::errorString(errnum)) << "\n"; 

namespace GL
{
   
   template<typename T>
   class _Exception : public std::exception
   {
   public:
      _Exception(const T& whatHappened)
      : _whatHappened(whatHappened) {}
      
      _Exception(const _Exception& te)
      : _whatHappened(te._whatHappened) {}
      
      _Exception& operator=(const _Exception& te)
      {
         if(this != &te)
         {
            _whatHappened = te._whatHappened;
         }
         return *this;
      }
      
      virtual ~_Exception() throw() {}
      
      virtual const char* what(void) const throw() = 0;
      
   protected:
      T _whatHappened;
      
   private:
      _Exception(void) {}
   };
   
   class Exception : public _Exception<std::string>
   {
   public:
      
      typedef _Exception<std::string> base_type;
      Exception(const std::string& wh) : base_type(wh) {}
      virtual const char* what(void) const throw() { return _whatHappened.c_str(); }
   };
   
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
      Program(const std::string& vertexFile, const std::string& fragmentFile, const std::string& geometryFile);
      
      /**
       * Destructor
       */
      ~Program();

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
      
      GLuint  _handle;         //< OpenGL handle for a GLSL shader
      Shader* _vertexShader;   //< Pointer to the fragment shader
      Shader* _fragmentShader; //< Pointer to the vertex shader
      Shader* _geometryShader; //< Pointer to the geometry shader
   };
}
#endif
