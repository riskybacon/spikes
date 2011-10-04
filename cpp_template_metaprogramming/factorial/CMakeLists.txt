# Require at least version 2.8 of CMake
cmake_minimum_required(VERSION 2.8)

# The project name. A project can have multiple targets
project(factorial)

# Add the executable target. It does not need to 
# have the same name as the project
add_executable(factorial # Name of the program
  main.cpp               # List of files, one on each line
)
