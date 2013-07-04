//
// OBJ model loader using OpenGL 3.2 / 4.1
//
// main.cpp
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>

#define _USE_MATH_DEFINES
#include <math.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // rotation, translation, scale, etc
#include <glm/gtc/type_ptr.hpp>         // value_ptr

// assimp include files
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace glm;
using std::vector;
using std::string;

// Comment this out to turn GL_ERR_CHECK into a no-op
//#define _DEBUG

#ifdef __APPLE__
#  define GLFW_INCLUDE_GLCOREARB
#else
// Non-Apple platforms need the GL Extension Wrangler
#  include <GL/glew.h>
#  if !defined(_WIN32) && !defined(_WIN64)
//  Assuming Unix, which needs glext.h
#    include <GL/glext.h>
#  endif
#endif

#include <shader.h>
#include <GLFW/glfw3.h>
#include "config.h"
#include "objmodel.h"

enum BUFFER_OBJECTS_ENUM
{
   VERTEX_BUFFER = 0,
   NORMAL_BUFFER,
   TEXCOORD_BUFFER,
   BUFFER_OBJECTS_NUM
};

// Global variables have an underscore prefix.
GL::Program*        _program;        //< GLSL program
GLuint              _vao;            //< Array object for the vertices
std::vector<GLuint> _buffer;         //< List of OpenGL buffers
bool                _running;        //< true if the program is running, false if it is time to terminate
bool                _tracking;       //< True if mouse location is being tracked
vector<vec4>        _vertexData;     //< Vertex data
vector<vec4>        _normalData;     //< Normal data
vector<vec2>        _tcData;         //< Texture coordinate data
std::string         _vertexFile;     //< Name of the vertex shader file
std::string         _fragFile;       //< Name of the fragment shader file
glm::mat4           _projection;     //< Camera projection matrix

// Window size
int                 _winWidth;       //< Width of the window
int                 _winHeight;      //< Height of the window

// Rotation
glm::quat           _objRot;         //< Eye rotation
glm::vec2           _prevCurPos;     //< Previous cursor pos
float               _sensitivity;    //< Sensitivity to mouse motion

// Log file
std::ofstream       _log;            //< Log file

// the global Assimp scene object
const aiScene*            _aiScene;

/**
 * Log an exception to stderr and to a log file
 *
 * @param exception
 *    The exception to log
 */
void logException(const std::runtime_error& exception)
{
	std::cerr << exception.what() << std::endl;
	_log << exception.what() << std::endl;
}

/**
 * Clean up and exit
 *
 * @param exitCode      The exit code, eg, EXIT_SUCCESS or EXIT_FAILURE
 */
void terminate(int exitCode)
{
   // Delete vertex buffer objects
   glDeleteBuffers(_buffer.size(), &_buffer[0]);

   // Delete vertex array object
   if(_vao)
   {
      glDeleteVertexArrays(1, &_vao);
   }
   
   glfwTerminate();

   exit(exitCode);
}

/**
 *  Initialize OpenGL Extension Wrangler. 
 *  Does nothing under OS X
 */
void initGLEW(void)
{
#ifndef __APPLE__
   // GLEW has trouble supporting the core profile
   glewExperimental = GL_TRUE;
   glewInit();
   while(glGetError());
   if(!GLEW_ARB_vertex_array_object)
   {
      std::cerr << "ARB_vertex_array_object not available." << std::endl; 
      terminate(EXIT_FAILURE);
   }
#endif
}

std::vector<aiVector3D> _objPos;
std::vector<aiVector3D> _objNormals;
std::pair<aiVector3D, aiVector3D> _objBoundingBox;
void addObjVertices(const aiScene* scene, const aiNode* node)
{
   for(int m = 0; m < node->mNumMeshes; m++)
   {
      const aiMesh* mesh = scene->mMeshes[node->mMeshes[m]];
      
      for(int f = 0; f < mesh->mNumFaces; f++)
      {
         const aiFace* face = &mesh->mFaces[f];
         for(int i = 0; i < face->mNumIndices; i++)
         {
            int index = face->mIndices[i];
            const aiVector3D& pos = mesh->mVertices[index];
            const aiVector3D& normal = mesh->mNormals[index];
            _objPos.push_back(pos);
            _objNormals.push_back(normal);
            
            if(_objPos.size() == 1)
            {
               _objBoundingBox.first = pos;
               _objBoundingBox.second = pos;
            }
            else
            {
               _objBoundingBox.first.x = _objBoundingBox.first.x > pos.x ? pos.x : _objBoundingBox.first.x;
               _objBoundingBox.first.y = _objBoundingBox.first.y > pos.y ? pos.y : _objBoundingBox.first.y;
               _objBoundingBox.first.z = _objBoundingBox.first.z > pos.z ? pos.z : _objBoundingBox.first.z;

               _objBoundingBox.second.x = _objBoundingBox.second.x < pos.x ? pos.x : _objBoundingBox.second.x;
               _objBoundingBox.second.y = _objBoundingBox.second.y < pos.y ? pos.y : _objBoundingBox.second.y;
               _objBoundingBox.second.z = _objBoundingBox.second.z < pos.z ? pos.z : _objBoundingBox.second.z;
            }
         }
      }
   }
   
   for(int n = 0; n < node->mNumChildren; n++)
   {
      addObjVertices(scene, node->mChildren[n]);
   }
}

aiNode
void readObj(const std::string& filename)
{
   _aiScene = aiImportFile(filename.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

   if(_aiScene == NULL)
   {
      std::cerr << "assimp fail" << std::endl;
   }
   

   const aiNode* rootNode = _aiScene->mRootNode;
   
   std::cout << "Num meshes: " << rootNode->mNumMeshes << std::endl;
   std::cout << "Num children: " << rootNode->mNumChildren << std::endl;
   
   addObjVertices(_aiScene, rootNode);
   
   float sizeX = _objBoundingBox.second.x - _objBoundingBox.first.x;
   float sizeY = _objBoundingBox.second.y - _objBoundingBox.first.y;
   float sizeZ = _objBoundingBox.second.z - _objBoundingBox.first.z;
   
   float scale = sizeX;
   
   scale = sizeX > sizeY ? sizeX : sizeY;
   scale = sizeZ > scale ? sizeZ : scale;
   
   scale = 1.0f / scale;
   
   for(int i = 0; i < _objPos.size(); i++)
   {
      _objPos[i] *= scale;
   }
}

/**
 * Initialize vertex array objects, vertex buffer objects,
 * clear color and depth clear value
 */
void init(void)
{
   try {
      initGLEW();
      
      int attribLoc;

      
      std::string objFile = std::string(SOURCE_DIR) + std::string("/frank_mesh_smooth.obj");
      //std::string objFile = std::string(SOURCE_DIR) + std::string("/teapot.obj");
      
      OBJModel* model = new OBJModel(objFile);
      model->unitize();
      
      readObj(objFile);
      
      
      GLuint mode = GLM_SMOOTH | GLM_TEXTURE;
      
      model->createBuffers(mode, _vertexData, _normalData, _tcData);
      _vertexFile = std::string(SOURCE_DIR) + "/vertex.c";
      _fragFile   = std::string(SOURCE_DIR) + "/fragment.c";
      
      _program = new GL::Program(_vertexFile, _fragFile);
      
      // Generate a single handle for a vertex array. Only one vertex
      // array is needed
      glGenVertexArrays(1, &_vao);
      _buffer.resize(BUFFER_OBJECTS_NUM);
      glGenBuffers(BUFFER_OBJECTS_NUM, &_buffer[0]);

      // Bind that vertex array
      glBindVertexArray(_vao);
      
      attribLoc = _program->getAttribLocation("vertex");
      if(attribLoc >= 0)
      {
         // Make that vbo the current array buffer. Subsequent array buffer operations
         // will affect this vbo
         //
         // It is possible to place all data into a single buffer object and use
         // offsets to tell OpenGL where the data for a vertex array or any other
         // attribute may reside.
         glBindBuffer(GL_ARRAY_BUFFER, _buffer[VERTEX_BUFFER]);
         GL_ERR_CHECK();
      
         // Set the data for the vbo. This will load it onto the GPU
         glBufferData(GL_ARRAY_BUFFER, _vertexData.size() * sizeof(glm::vec4),
                      &_vertexData[0], GL_STATIC_DRAW);
      
         glBufferData(GL_ARRAY_BUFFER, _objPos.size() * sizeof(aiVector3D), &_objPos[0], GL_STATIC_DRAW);
         
         // Specify the location and data format of the array of generic vertex attributes
         glVertexAttribPointer(attribLoc,  // Attribute location in the shader program
                               3,          // Number of components per attribute
                               GL_FLOAT,   // Data type of attribute
                               GL_FALSE,   // GL_TRUE / GL_FALSE: values are normalized true/false
                               0,          // Stride
                               0);         // Offset into currently bound array buffer for this data
      
         // Enable the generic vertex attribute array
         glEnableVertexAttribArray(attribLoc);
         GL_ERR_CHECK();
      }

      attribLoc = _program->getAttribLocation("normal");
      if(attribLoc >= 0)
      {
         // Set up normal attribute
         glBindBuffer(GL_ARRAY_BUFFER, _buffer[NORMAL_BUFFER]);
      
         glBufferData(GL_ARRAY_BUFFER, _normalData.size() * sizeof(glm::vec4),
                      &_normalData[0], GL_STATIC_DRAW);
      
         glBufferData(GL_ARRAY_BUFFER, _objNormals.size() * sizeof(aiVector3D), &_objNormals[0], GL_STATIC_DRAW);
         glVertexAttribPointer(attribLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(attribLoc);
         GL_ERR_CHECK();
      }

      attribLoc = _program->getAttribLocation("tc");
      if(attribLoc >= 0)
      {
         // Set up texture attribute
         glBindBuffer(GL_ARRAY_BUFFER, _buffer[TEXCOORD_BUFFER]);
      
         glBufferData(GL_ARRAY_BUFFER, _tcData.size() * sizeof(glm::vec2),
                      &_tcData[0], GL_STATIC_DRAW);
      
         glVertexAttribPointer(_program->getAttribLocation("tc"), 2, GL_FLOAT, GL_FALSE, 0, 0);
         glEnableVertexAttribArray(_program->getAttribLocation("tc"));
         GL_ERR_CHECK();
      }

      // Set the clear color
      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      
      // Set the depth clearing value
      glClearDepth(1.0f);
      
      // Enable depth test
      glEnable(GL_DEPTH_TEST);
   } 
   catch (std::runtime_error exception)
   {
      logException(exception);
      terminate(EXIT_FAILURE);
   }

}

void reloadShaders(void)
{
   try
   {
      GL::Program* newProgram = new GL::Program(_vertexFile, _fragFile);
      delete _program;
      _program = newProgram;
      
   }
   catch (std::runtime_error exception)
   {
      logException(exception);
   }
}
/**
 * Window resize callback
 *
 * @param width   the width of the window
 * @param height  the height of the window
 */
void resize(GLFWwindow* window, int width, int height)
{
   try
   {
      // Set the affine transform of (x,y) from normalized device coordinates to
      // window coordinates. In this case, (-1,1) -> (0, width) and (-1,1) -> (0, height)
      glViewport(0, 0, width, height);
      GL_ERR_CHECK();
      
      _winWidth = width;
      _winHeight = height;
      
      // Projection matrix
      _projection = glm::perspective(45.0f,                        // 45 degree field of view
                                     float(width) / float(height), // Ratio
                                     0.1f,                         // Near clip
                                     100.0f);                     // Far clip
   }
   catch(std::runtime_error exception)
   {
      logException(exception);
      terminate(EXIT_FAILURE);
   }
}

/**
 *  Mouse click callback
 *
 *  @param window  pointer to the window being resized
 *  @param button that was clicked
 *  @param button state
 */
void mouseButton(GLFWwindow* window, int button, int action, int mods)
{
   if(button == GLFW_MOUSE_BUTTON_1)
   {
      if(action == GLFW_PRESS)
      {
         _tracking = true;
         double x, y;
         glfwGetCursorPos(window, &x, &y);
         _prevCurPos = vec2(x,y);
      }
      else
      {
         _tracking = false;
      }
   }
}

/**
 * Mouse movement callback
 */
void cursorPos(GLFWwindow* window, double x, double y)
{
   if(_tracking)
   {
      // Get the change in position
      vec2 curPos(x,y);
      vec2 delta = curPos - _prevCurPos;
      _prevCurPos = curPos;
      
      
      vec3 eulerY = vec3(0, 1, 0) * delta.x * _sensitivity;
      vec3 eulerX = vec3(1, 0, 0) * delta.y * _sensitivity;
      
      glm::quat xRot = quat(eulerX);
      glm::quat yRot = quat(eulerY);

      _objRot = normalize(yRot * xRot * _objRot);
   }
}

/**
 * Keypress callback
 */
void keypress(GLFWwindow* window, int key, int scancode, int state, int mods)
{
   if(state == GLFW_PRESS)
   {
      switch(key)
      {
         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
      }
   }
}

/**
 * Window close callback
 */
void close(GLFWwindow* window)
{
   glfwSetWindowShouldClose(window, GL_TRUE);
}
/**
 * Main loop
 * @param time    time elapsed in seconds since the start of the program
 */
void render(double time)
{
   try
   {
      // Clear the color and depth buffers
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Camera matrix
     glm::mat4 view       = glm::lookAt(glm::vec3(0,0,5), // Camera position is at (4,3,3), in world space
                                        glm::vec3(0,0,0), // and looks at the origin
                                        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                       );

      // Model matrix
      glm::mat4 model = glm::mat4_cast(_objRot);
   
     // Create  model, view, projection matrix
     glm::mat4 mvp        = _projection * view * model; // Remember, matrix multiplication is the other way around
   
     // Calculate the inverse transpose for use with normals
     glm::mat4 invTP      = transpose(glm::inverse(mvp));

     // Use the shader program that was loaded, compiled and linked
     _program->bind();
   
     // Set the MVP uniform
     _program->setUniform("mvp", mvp);
   
     // Set the inverse transpose uniform
     _program->setUniform("invTP", invTP);

     // Draw the triangles
      //     glDrawArrays(GL_TRIANGLES, 0, _vertexData.size());
     glDrawArrays(GL_POINTS, 0, _objPos.size());
      
     GL_ERR_CHECK();
   }
   catch(std::runtime_error err)
   {
      logException(err);
      terminate(EXIT_FAILURE);
   }

}

/**
 * Program entry point
 */
int main(int argc, char* argv[])
{
   int width = 1024; // Initial window width
   int height = 768; // Initial window height
   _sensitivity = float(M_PI) / 360.0f;
   
   // Open up the log file
   std::string logFile = std::string(PROJECT_BINARY_DIR) + "/log.txt";
   _log.open(logFile.c_str());
   
   GLFWwindow* window;
   
   // Initialize GLFW
   if(!glfwInit())
   {
      return EXIT_FAILURE;
   }
   
   // Request an OpenGL core profile context, without backwards compatibility
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
   glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
   glfwWindowHint(GLFW_SAMPLES,               8);
   glfwWindowHint(GLFW_RED_BITS,              32);
   glfwWindowHint(GLFW_GREEN_BITS,            32);
   glfwWindowHint(GLFW_BLUE_BITS,             32);
   glfwWindowHint(GLFW_ALPHA_BITS,            32);
   
#ifdef __APPLE__
   // This should never be needed and it is recommended to never set this bit,
   // but OS X requires it to be set to get a core profile.
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
   
   // Create a windowed mode window and its OpenGL context
   window = glfwCreateWindow(1024, 768, "objreader", NULL, NULL);
   if(!window)
   {
      std::cerr << "Failed to open GLFW window" << std::endl;
      glfwTerminate();
      return EXIT_FAILURE;
   }
   
   glfwSetKeyCallback(window,         keypress);
   glfwSetMouseButtonCallback(window, mouseButton);
   glfwSetWindowSizeCallback(window,  resize);
   glfwSetWindowCloseCallback(window, close);
   glfwSetCursorPosCallback(window,   cursorPos);
   
   // Make the window's context current
   glfwMakeContextCurrent(window);
   
   printf("GL Version: %s\n", glGetString(GL_VERSION));
   
   init();
   resize(window, width, height);
   
   // Loop until the user closes the window
   while(!glfwWindowShouldClose(window))
   {
      // Render scene
      render(glfwGetTime());
      
      // Swap front and back buffers
      glfwSwapBuffers(window);
      
      // Poll for and process events
      glfwPollEvents();
   }
   
   glfwTerminate();
   
   terminate(EXIT_SUCCESS);
}