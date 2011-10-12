#----------------------------------------------------------------------
# 
#----------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8)

set(PROJ_NAME objreader)

project(${PROJ_NAME})

# Include the platform specific configuration.
# This will define the following useful variables:
#
# LIBRARIES	The libraries to be linked to the executable
# INCLUDE_PATH	Path to the include files
include(${CMAKE_SOURCE_DIR}/PlatformSpecifics.cmake)

# Set the include directories
include_directories(${INCLUDE_PATH})

# Get the path to the source code and create a define. This is used
# for locating the shaders
add_definitions("-DSOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

# Add a target executable
add_executable(${PROJ_NAME}
  main.cpp
  glm.c
  objmodel.cpp
  trackball.cpp
  glm.h
  objmodel.h
  platform_specific.h
  trackball.h
)

# Libraries to be linked
target_link_libraries(${PROJ_NAME}
  ${LIBRARIES}
)
