1. Purpose: 

Builds on the triangle demo by introducing a uniform variable for
the model/view/projection matrix. Rotates the triangle about the
y-axis once every second.

2. Dependencies:

cmake-2.8.4 (to create Makefile or project files)
glfw-2.7.2 (http://www.glfw.org/download.html)
glm-0.9.2 (http://glm.g-truc.net)
glew (1.5 or greater) http://glew.sourceforge.net/ (Windows and Linux only)

3. Building GLFW under OS X Lion

Follow the instructions in ../lion_glfw_patch

4. Building the demo under OS X or Linux:
mkdir build
cd build
cmake ..
make
./cube41

5. To build a debug version under Linux or OS X:

cmake "-DCMAKE_BUILD_TYPE:STRING=Debug" ..
make
./cube41

6. Building on Windows:

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

Place glm into

C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include\glm
