Hello world, built using CMake. This shows how to
specify the Intel C++ compiler.

Build using:

source /opt/intel/bin/iccvars.sh intel64
mkdir build
cd build
ccmake ..
[ press c to configure, press c again, then g to generate ]
make
./hello_world

To build an XCode project:

source /opt/intel/bin/iccvars.sh intel64
mkdir build
cd build
ccmake -G Xcode ..
[ press c to configure, press c again, then g to generate ]
open hello_world.xcodeproj
[ press build and run ]

