cmake_minimum_required(VERSION 2.8)

set(PROJ_NAME freeimage_texture)

project(${PROJ_NAME})

# Include the platform specific configuration.
# This will define the following useful variables:
#
# LIBRARIES	The libraries to be linked to the executable
# INCLUDE_PATH	Path to the include files
include(${CMAKE_SOURCE_DIR}/PlatformSpecifics.cmake)

set(SHADER_SOURCE_DIR
  ${CMAKE_SOURCE_DIR}/../shader
)

set(INCLUDE_PATH ${INCLUDE_PATH} ${SHADER_SOURCE_DIR} ${PROJECT_BINARY_DIR})

# Set the include directories
include_directories(${INCLUDE_PATH})

# OpenGL core context version
set (GL_MAJOR 3)
set (GL_MINOR 2)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

set(SOURCE_FILES
  main.cpp
  ${SHADER_SOURCE_DIR}/shader.cpp
)

set(HEADER_FILES
  ${SHADER_SOURCE_DIR}/shader.h
)

set(SHADER_FILES
  texture.vsh
  texture.fsh
)


# Add a target executable
add_executable(${PROJ_NAME}
  ${SOURCE_FILES}
  ${HEADER_FILES}
  ${SHADER_FILES}
)

# Libraries to be linked
target_link_libraries(${PROJ_NAME}
  ${LIBRARIES}
)
