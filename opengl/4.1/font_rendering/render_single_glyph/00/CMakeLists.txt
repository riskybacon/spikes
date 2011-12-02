cmake_minimum_required(VERSION 2.8)

set(PROJ_NAME render_single_glyph)

project(${PROJ_NAME})

# Include the platform specific configuration.
# This will define the following useful variables:
#
# LIBRARIES	The libraries to be linked to the executable
# INCLUDE_PATH	Path to the include files
include(${CMAKE_SOURCE_DIR}/PlatformSpecifics.cmake)

find_path(FREETYPE_INCLUDE_PATH freetype/freetype.h
  /opt/local/include/freetype2
)

find_path(FT2BUILD_INCLUDE_PATH ft2build.h
  /opt/local/include
)

find_library(FREETYPE_LIBRARY freetype
  /opt/local/lib
)

# Set the include directories
include_directories(
  ${INCLUDE_PATH}
  ${FREETYPE_INCLUDE_PATH}
  ${FT2BUILD_INCLUDE_PATH}
)

find_library(FREETYPE_LIBRARY freetype)

# Get the path to the source code and create a define. This is used
# for locating the shaders
add_definitions("-DSOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

# Add a target executable
add_executable(${PROJ_NAME}
  font.cpp
  font.h
  main.cpp
  shader.cpp
  trackball.cpp
  shader.h
  opengl.h
  platform_specific.h
  trackball.h
)

# Libraries to be linked
target_link_libraries(${PROJ_NAME}
  ${LIBRARIES}
  ${FREETYPE_LIBRARY}
)
