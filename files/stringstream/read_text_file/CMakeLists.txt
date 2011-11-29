# Require at least version 2.8 of CMake
cmake_minimum_required(VERSION 2.8)

# The project name. A project can have multiple targets
project(stringstream)

# Get the path to the source code and create a define. This is used
# for locating the input file
add_definitions("-DSOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

# Add the executable target. It does not need to 
# have the same name as the project
add_executable(stringstream    # Name of the program
  main.cpp                     # List of files, one on each line
)
