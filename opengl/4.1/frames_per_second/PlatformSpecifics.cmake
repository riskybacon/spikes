#------------------------------------------------------------------------------
# Sets up the platform specific aspects of the build
#
# Variables set:
#
# LIBRARY_SEARCH_PATH	Paths to search for libraries
# HEADER_SEARCH_PATH	Paths to search for header files
#
# LIBRARIES		Libraries that need to be linked into the executable
# INCLUDE_PATH          Paths to the required include directories
#------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8)

# Set up platform specific search paths
if(APPLE)

  set(LIBRARY_SEARCH_PATH
    /usr/local
    /opt/local
  )

  set(HEADER_SEARCH_PATH
    /usr/local
    /opt/local
  )

else(APPLE)

  if(WIN32) # Also true on windows 64-bit

    set(LIBRARY_SEARCH_PATH
      "C:/Program Files (x86)/Microsoft SDKs/Windows/v8.0A/Lib"
      "C:/Program Files/Microsoft SDKs/Windows/v8.0a/Lib"
      "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Lib"
      "C:/Program Files/Microsoft SDKs/Windows/v7.0a/Lib"
    )

    set(HEADER_SEARCH_PATH
      "C:/Program Files (x86)/Microsoft SDKs/Windows/v8.0a/Include"
      "C:/Program Files/Microsoft SDKs/Windows/v8.0a/Include"
      "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0a/Include"
      "C:/Program Files/Microsoft SDKs/Windows/v7.0a/Include"
    )

  else(WIN32)

    if(UNIX)

      set(LIBRARY_SEARCH_PATH
        /usr/local
        /opt/local
        /usr
        /opt
      )

      set(HEADER_SEARCH_PATH
        /usr/local
        /opt/local
        /usr
        /opt
      )

    else(UNIX)

      # Oh noes!
      message(WARNING "Supported platforms: Unix, OS X and Windows. Your platform isn't supported, but you can still try and build.")
      message(WARNING "Make sure and set LIBRARY_SEARCH_PATH and HEADER_SEARCH_PATH")
    endif(UNIX)
  endif(WIN32)
endif(APPLE)

# Find OpenGL
find_package(OpenGL)

# Find glfw
find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h ${HEADER_SEARCH_PATH})
find_library(GLFW_LIBRARIES glfw3 ${LIBRARY_SEARCH_PATH})


if(WIN32)
  set( ENV{FREETYPE_DIR} "C:/Program Files (x86)/freetype" )
endif(WIN32)

find_package(Freetype)

# Include directories for this project
set(INCLUDE_PATH
  ${OPENGL_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIR}
  ${FREETYPE_INCLUDE_DIRS}
)


# Libraries needed on all platforms for this project
set(LIBRARIES
  ${OPENGL_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${FREETYPE_LIBRARIES}
)

# Platform specific libraries and header directories
if(APPLE)

  # Add OS X specific libraries
  set(LIBRARIES ${LIBRARIES}
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreFoundation"
  )

else(APPLE)

  # Handle Linux specific dependencies
  if(WIN32)
    set(LIBRARIES ${LIBRARIES}
      "winmm"
    )

  else(WIN32)

    # Find Xrandr - needed by GLFW under Unix
    find_library(XRANDR_LIBRARY Xrandr
      ${LIBRARY_SEARCH_PATH}
    )

    set(LIBRARIES ${LIBRARIES}
      ${XRANDR_LIBRARY}
    )

  endif(WIN32)

  # Windows and Linux need GLEW, the OpenGL Extension Wrangler
  find_path(GLEW_INCLUDE_DIR GL/glew.h
    ${HEADER_SEARCH_PATH}
  )

  set(INCLUDE_PATH ${INCLUDE_PATH}
    ${GLEW_INCLUDE_DIR} 
  )

  find_library(GLEW_LIBRARY glew32
    ${LIBRARY_SEARCH_PATH}
  )

  set(LIBRARIES ${LIBRARIES}
    ${GLEW_LIBRARY}
  )

endif(APPLE)
