Building GLFW under OS X Lion

Under OS X Lion, glfw-2.7.2  OpenGL 3.2 core profile
support is not fully baked. The problem is the it does
not include the OpenGL/gl3.h header file. No big deal,
there's an easy fix that does not impact any other
software on your system that may use this library.

When installing glfw, follow this procedure:

tar zxf glfw-2.7.2.tar.gz
cd glfw-2.7.2
patch -p1 < /path/this/patch/lion_ogl32.patch 
sudo make cocoa-install

You will need to replace /path/to/this/patch with the
real path to the patch.

The library will be installed in /usr/local

To enable OpenGL 3 support, put a #define GLFW_GL3 
prior to including <GL/glfw.h> in your code. This has
already been done in these example.
