Hello world, built using CMake. Build using:

mkdir build
cd build
ccmake ..
[ press c to configure, press c again, then g to generate ]
make
./hello_world

To build an XCode project:

mkdir build
cd build
ccmake -G Xcode ..
[ press c to configure, press c again, then g to generate ]
open hello_world.xcodeproj
[ press build and run ]

