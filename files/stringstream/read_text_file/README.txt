Demonstrates the use of stringstreams in reading
in a file and performing some simple parsing. This
spike was written to be a basis for reading OBJ files

To build:

mkdir build
cd build
ccmake ..
[ press c to configure, press c again, then g to generate ]
make
./stringstream

To build an XCode project:

mkdir build
cd build
ccmake -G Xcode ..
[ press c to configure, press c again, then g to generate ]
open stringstream.xcodeproj
[ press build and run ]

