/**
 * Demonstrates how to read in a file using C++ and without
 * copying the contents inside your program. Most examples
 * show the file being read line by line and then copied
 * into a buffer.
 *
 * This example show how to allocate a buffer of the correct
 * length and then use that buffer within a std::stringstream
 * so that the file is read just once.
 *
 * Since the file will be buffered by the kernel, a copy will
 * be made when the data is copied out of kernel space and into
 * the program's space, but that is the only copy that will be
 * occcuring.
 *
 * The method used for error reporting is not thread safe. In
 * practice, this will usually not matter and will work as 
 * expected in a multithreaded program.
 *
 * The reason for this is the use of the "errno" variable, which
 * is a global variable and is not tied directly to a particular
 * file.
 *
 * TODO: 
 * 1) Should an exception of type std::runtime_error be thrown,
 *    or should it be subclassed and the subclass thrown?
 * 2) Verify functionality under Windows and Linux. Make sure that
 *    the following error conditions can be found
 *    . File not found
 *    . Permission denied
 *    . File locked by other process
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <sys/errno.h>

using std::string;
using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::vector;

/**
 * The contents of a file. This class ties
 * together a storage object and a stringstream
 * to store the file contents. It is possible to
 * let stringstream handle all of the storage, but
 * then an extra copy will be made in user space.
 *
 * This class will not lock the file. It is designed
 * to read the contents of a text file into memory. It
 * is not designed to lock and update a file.
 */
class TextFile {
public:
   /**
    * Constructor. Attempts to open a text file and read
    * it into a stringstream
    *
    * @param filename
    *    The file to be read
    */
   TextFile(const std::string& filename);

   /**
    * @return the stringstream associated with this file. If a
    *    string containing the file is needed, use the following:
    *    std::string contents = myFile.getStringStream().str();
    *    Beware, this will result in a copy. To get the string
    *    without the copy, use a reference. 
    *    const std::string& contents = myFile.getStringStream().str();
    *    The problem with a reference is that if the TextFile
    *    object goes out of scope, the reference will no longer
    *    be valid and if used may result in a segfault, or worse,
    *    may not result in a segfault and will cause the program
    *    to read memory it should not be reading!
    */
   std::stringstream& getStringStream(void) { return mStream; }

private:
   std::vector<char> mBuffer; //< Storage for the file
   std::stringstream mStream; //< Stringstream for the file
};

// Constructor
TextFile::TextFile(const std::string& filename) {
   ifstream file;
   file.exceptions ( ifstream::failbit | ifstream::badbit );

   try {
      // Open the file
      file.open(filename.c_str());
      // Get the size of the file
      file.seekg(0,std::ios::end);
      std::streampos length = file.tellg();
      file.seekg(0,std::ios::beg);
      
      // Use a vector of char as the buffer. It is exception
      // safe and will be cleaned up when the destructor is called.
      // It is resized to be the same length as the file
      mBuffer.resize(length);

      // Read the whole file into the buffer
      file.read(&mBuffer[0],length);
      
      // Tell the stringstream to use the previously defined buffer
      // as its storage
      mStream.rdbuf()->pubsetbuf(&mBuffer[0],length);

      // The buffer is not copied, like in most C++ example programs.
      // The buffer will be cleaned up when this TextFile object goes
      // out of scope.
      file.close();
   } catch(ifstream::failure& e) {
      // This is probably not thread safe. The errno variable is a global
      // variable and not associated with any particular file. Unfortunately,
      // the error information that the standard library streams reports
      // for error conditions is not helpful for creating useful error
      // messages. It is unusual for a program to be opening multiple
      // files at the same time in multiple threads, so this should work
      // most of the time.
      throw(std::runtime_error(strerror(errno)));
   }
}

int main(int, char**)
{
   string filename = string(SOURCE_DIR) + "/cube.obj";
   try {
      TextFile file(filename);
      cout << file.getStringStream().str();
   } catch(std::runtime_error& e) {
      std::cerr << "Error reading file " << filename << " : " << e.what() << std::endl;
      exit(EXIT_FAILURE);
   }
   exit(EXIT_SUCCESS);
}
