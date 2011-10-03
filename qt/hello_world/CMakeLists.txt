# Minimum version of CMake
cmake_minimum_required(VERSION 2.8)

# Project name
set(PROJ_NAME hello_world)

# Set the project name
project(${PROJ_NAME})

# Include the FindQt module. The user will probably
# need provide a path to qmake during configuration
include(${CMAKE_ROOT}/Modules/FindQt4.cmake) 

# List of C/C++ files
set(SOURCE_FILES
  main.cpp
)

# List of header files
set(HEADER_FILES
)

# List of directories for header files
include_directories(
   ${QT_INCLUDES}
)

# Add the executable target. It does not need to 
# have the same name as the project
add_executable(${PROJ_NAME}
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

# Libraries to link to the target
target_link_libraries(${PROJ_NAME}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTCORE_LIBRARY}
)



