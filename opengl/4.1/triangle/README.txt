Purpose: 

To demonstrate obtaining and using an OpenGL 3.2 or 4.1 core profile
context that does not offer backward compatibility. A window is created
and a single red triangle is drawn to show that a simple shader program
can be loaded and used. 

This example is written entirely in C and all code is in a single file

Dependencies:

cmake-2.8.4 (to create Makefile or project files)
glfw-3.0 (http://www.glfw.org/download.html)
glew (1.5 or greater) http://glew.sourceforge.net/ (Windows and Linux only)

Building GLFW under OS X Lion

When installing glfw, follow this procedure:

tar zxf glfw-2.7.2.tar.gz
cd glfw-2.7.2
patch -p1 < ../lion_ogl32.patch 
sudo make cocoa-install

This will install the library in /usr/local

Building the demo under OS X or Linux:
mkdir build
cd build
cmake ..
make
./triangle

To build a debug version under Linux or OS X:

cmake "-DCMAKE_BUILD_TYPE:STRING=Debug" ..
make
./glfw272


Building on Windows:

You will need to install GLEW and GLFW. This process isn't nicely 
automated, so you will need to put the libraries and headers into
place manually. These instructions show how to use the default
locations set up in PlatformSpecifics.cmake, but they can be 
installed anywhere you have permissions

Place glew32.lib and GLFW.lib into:
C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Lib

Place glew32.dll and GLFW.dll into:

C:\Windows\System32

Place glew.h and glfw.h into:

C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include\gl

