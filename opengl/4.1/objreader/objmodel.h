//----------------------------------------------------------------------
// OBJModel.h
//
// Jeff Bowles <jbowles@riskybacon.com>
//----------------------------------------------------------------------

#ifndef _OBJModel_h
#define _OBJModel_h

#include <ostream>
#include <string>
#include <vector>

#if defined(__APPLE_CC__)
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>

extern "C" {
#include "glm.h"
}

//----------------------------------------------------------------------
/// Wrapper class for Nate Robbin's glm OBJ file handling library. Adds
/// the ability to find seams in a model.
///
/// To understand how seams are found, check the OBJModel.cpp file. 
/// The methods primarily involved in the process are the constructor,
/// findSeams() and findMatchingSeams() .
///
/// How to use this class
/// \code
/// // Create the model
/// OBJModel* _model = new OBJModel("filename.obj");
///
/// // Find the seams
/// _model->findSeams();
///
/// // Draw the seams - specify a point size / line size 
/// _model->drawSeams(3.0f)
///
/// // Draw the model
/// _model->draw();
/// \endcode
//----------------------------------------------------------------------
class OBJModel
{
public:
   ///
   /// Constructor
   /// 
   /// \param filename Name of the OBJ file
   ///
   OBJModel(const std::string&);
   
   ///
   /// Destructor
   ///
   ~OBJModel();

   ///
   /// Create buffers for use in glDrawArrays
   void createBuffers(GLuint mode,
                      std::vector<glm::vec4>& vertices,
                      std::vector<glm::vec4>& normals,
                      std::vector<glm::vec2>& texcoords);

   ///
   /// Generate normals for each facet. Use this if the model does not
   /// have any normals. This will result in a flat shaded model as there
   /// will only be one normal per triangle
   void facetNormals(void);
   
   ///
   /// Generate normals for each vertex. Normals a generated for
   /// each triangle for which the vertex is a member and averaged. Useful
   /// for creating a smoothly shaded model when no normals are provided.
   ///
   void vertexNormals(float angle);
   
   ///
   /// Unitize a model by translating it to the origin and
   /// scaling it to fit in a unit cube around the origin.
   /// \return the scalefactor used to unitize the model.
   ///
   GLfloat unitize(void) { return glmUnitize(_model); }
   
   void reverseWinding(void) { glmReverseWinding(_model); } 
   
   //------------------------------------------------------------------
   // Helper functions for finding seams
   //------------------------------------------------------------------
   
private:
   
   ///
   /// Point to the wrapped model.
   GLMmodel* _model;
};

#endif

