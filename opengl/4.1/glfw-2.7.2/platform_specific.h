//
// platform_specifics.h
//
// Platform specific includes and defines.
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//
// Changelog: 
// 2011-09-02, Initial revision <jbowles@riskybacon.com>
//

#ifndef _PLATFORM_SPECIFIC_H
#define _PLATFORM_SPECIFIC_H

#if defined(_WIN32) || defined(_WIN64)
// Visual Studio upconverts const char* to std::string when doing
// the following:
//   std::cerr << "blah blah" << std::endl;
// For this reason, the string header must be included.
#include <string>
#endif

#ifndef __APPLE__
// Non-Apple platforms need the GL Extension Wrangler
#include <GL/glew.h>
#  if !defined(_WIN32) && !defined(_WIN64)
//  Assuming Unix, which needs glext.h
#  include <GL/glext.h>
#  endif
#else
// Make sure GLFW knows to include gl3.h header under OS X. This
// requires the GL/glfw.h be patched, otherwise it will include gl.h
// and the output from this program will be a black screen. For this
// to work, glfw.h must be patched. See README.txt for details.
#define GLFW_GL3
#endif


// Which version of OpenGL to use? Under Apple, use 3.2.
// Otherwise, use 4.1
#ifdef __APPLE__
#  define GL_MAJOR 3
#  define GL_MINOR 2
#else
#  define GL_MAJOR 4
#  define GL_MINOR 1
#endif

#endif