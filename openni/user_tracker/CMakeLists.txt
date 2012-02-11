# Require at least version 2.8 of CMake
cmake_minimum_required(VERSION 2.8)

# Set the project name
set(PROJ_NAME user_tracker)

# The project name. A project can have multiple targets
project(${PROJ_NAME})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
 ${CMAKE_SOURCE_DIR}/../cmake/
)

include(FindOpenNI)
include(FindOpenGL)
include(FindGLFW)

# Make sure that OpenNI is found
if(NOT OpenNI_FOUND)
  message(ERROR "Could not find OpenNI")
endif(NOT OpenNI_FOUND)

# Make sure that OpenGL is found
if(NOT OPENGL_FOUND)
  message(ERROR "Could not find OpenGL")
endif(NOT OPENGL_FOUND)

# Make sure that GLFW is found
if(NOT GLFW_FOUND)
  message(ERROR "Could not find GLFW")
endif(NOT GLFW_FOUND)

# Get the path to the source code and create a define. This is used
# for locating the shaders
add_definitions("-DSOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
add_definitions("-DCONFIG_FILE=\"${CMAKE_SOURCE_DIR}/../config/samples_config.xml\"")
# Set the include directories
include_directories(
  ${OpenNI_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIR}
)

# Add the executable target. It does not need to 
# have the same name as the project
add_executable(${PROJ_NAME}  # Name of the program
  main.cpp                   # List of files, one on each line
  SceneDrawer.cpp
  SceneDrawer.h
  opengles.cpp
  opengles.h
)

target_link_libraries(${PROJ_NAME}
  ${OpenNI_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLFW_LIBRARIES}
)
