#pragma once

#define OPENGL3

#include "platform_specific.h"

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
