//----------------------------------------------------------------------
// OBJModel.cpp
//
// Wrapper for Nate Robbin's glm code to read in Wavefront OBJ files
//----------------------------------------------------------------------

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "objmodel.h"

using std::cout;
using std::endl;

#define T(x) (_model->triangles[(x)])

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------
OBJModel::OBJModel(const std::string& filename)
{
   //------------------------------------------------------------------
   
   // This is totally lame, but glm needs a char*, not a const char*
   // to be passed in which leads to this ugly conversion method.
   // Notice that no check is made to see if the malloc succeeded.
   size_t length = filename.length();
   char* fn = (char*) malloc(sizeof(char) * (length + 1));

   filename.copy(fn, length, 0);
   fn[length] = '\0'; // Add NULL terminator to the string
   cout << "Reading OBJ file " << fn << endl;
   _model = glmReadOBJ(fn);
   free(fn);
   
   cout << "done reading obj model" << endl;
}



void OBJModel::createBuffers(GLuint mode,
                             std::vector<glm::vec4>& vertices,
                             std::vector<glm::vec4>& normals,
                             std::vector<glm::vec2>& texcoords
                             )
{
   GLuint i;
   GLuint j;
   GLMgroup* group = _model->groups;
   GLMtriangle* triangle;
   
   vertices.clear();
   normals.clear();
   texcoords.clear();
   
   for (i = 0; i < group->numtriangles; i++) {
      triangle = &T(group->triangles[i]);
      GLfloat* ptr;
      if (mode & GLM_FLAT)
      {
         // glNormal3fv(&model->facetnorms[3 * triangle->findex]);
         ptr = &_model->facetnorms[3 * triangle->findex];
         for(j = 0; j < 3; ++j)
         {
            normals.push_back(glm::vec4(*ptr, *(ptr + 1), *(ptr + 2), 0.0f));
         }
      }
      
      if (mode & GLM_SMOOTH)
      {
         //         glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
         for(j = 0; j < 3; ++j)
         {
            ptr = &_model->normals[3 * triangle->nindices[j]];
            normals.push_back(glm::vec4(*ptr, *(ptr + 1), *(ptr + 2), 0.0f));
         }
      }
      if (mode & GLM_TEXTURE)
      {
         //         glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);
         for(j = 0; j < 3; ++j)
         {
            ptr = &_model->texcoords[2 * triangle->tindices[j]];
            texcoords.push_back(glm::vec2(*ptr, *(ptr + 1)));
         }
      }
      
      for(j = 0; j < 3; ++j)
      {
         //      glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);
         ptr = &_model->vertices[3 * triangle->vindices[j]];
         vertices.push_back(glm::vec4(*ptr,
                                      *(ptr + 1),
                                      *(ptr + 2),
                                      1.0f));
      }
   }
}


void OBJModel::facetNormals(void)
{
   glmFacetNormals(_model);
}

void OBJModel::vertexNormals(float angle)
{
   glmVertexNormals(_model, angle);
}

OBJModel::~OBJModel()
{
   glmDelete(_model);
}