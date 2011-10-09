//----------------------------------------------------------------------
// Virtual Trackball
//
// Math by Ed Angel from OpenGL: A Primer
//
// Implementation by Jeff Bowles <jbowles@riskybacon.com>
//
// Usage:
//
// Create a new instance - multiple instances are possible
// Trackball* _trackball = new Trackball(windowWidth, windowHeight)
//
// After a window reshape event:
// _trackball->reshape(windowWidth, windowHeight);
//
// When mouse button is pressed:
// _trackball->start(mouseX,mouseY);
//
// When mouse is in motion:
// _trackball->motion(mouseX, mouseY);
//
// When mouse button is released:
// _trackball->stop();
//
// To apply the trackball rotation transform:
// glMatrixMode(GL_MODELVIEW);
// ...apply other transforms if needed
// _trackball->applyTransformation();
//----------------------------------------------------------------------
#include "Trackball.h"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------
Trackball::Trackball(int width, int height)
: _tracking(false)
{
   // Set the width and height
   reshape(width, height);
   
   // Set transform to the identity
   reset();
   
   // Initialize previous position
   _prevPos[0] = 0.0f;
   _prevPos[1] = 0.0f;
   _prevPos[2] = 0.0f;
}

//----------------------------------------------------------------------
// Reset the trackball transform to the identity matrix
//----------------------------------------------------------------------
void Trackball::reset(void)
{
   _trans = glm::mat4(1.0f);
}

//----------------------------------------------------------------------
// Change the size of the window
//----------------------------------------------------------------------
void Trackball::reshape(int width, int height)
{
   _width = width;
   _height = height;
}

//----------------------------------------------------------------------
// Project the x & y mouse coordinates onto the trackball sphere.
// The coordinates are in v[3]
//----------------------------------------------------------------------
glm::vec3 Trackball::projection(int x, int y)
{
   glm::vec3 v;
   
   // Window coordinates for y need to be flipped
   y = _height - y;
   
   // Scale and bias (x,y) window coordinates into (-1, 1) range
   v[0] = (2.0f * x - _width ) / _width;
   v[1] = (_height - 2.0f * y ) / _height;
   
   // Find distance between v[0] and v[1]
   float d = sqrt(v[0] * v[0] + v[1] * v[1]);
   
   // Clamp d to [0, 1]
   d = (d < 1.0f ? d : 1.0f);
   
   
   v[2] = cos((M_PI / 2.0f) * d);
   
   // The trackball is represented by a unit sphere, so the
   // vector needs to be normalized to be on the sphere.
   float a = 1.0f / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
   v[0] *= a;
   v[1] *= a;
   v[2] *= a;
   
   return v;
}

//----------------------------------------------------------------------
// Handle trackball motion events. This calculates a new
// transformation matrix than can be applied using applyTransform()
//----------------------------------------------------------------------
void Trackball::motion(int x, int y)
{
   // Only calculate a new transform if motion is being tracked
   if(_tracking)
   {
      glm::vec3 curPos;
      glm::vec3 delta;
      
      // Project the mouse window space coordinates into trackball space
      curPos = projection(x, y);
      
      // Find the change in position
      delta = curPos - _prevPos;
      // Compute distance of the change
      float deltaLen = glm::length(delta);
      
      // If the change is really small, then don't bother updating
      // the transformation.
      if(deltaLen > 0.00001)
      {
         float angle = 90 * deltaLen;
         glm::vec3 axis;
         // Cross product of prevPos and curPos
         axis = glm::cross(_prevPos, curPos);

         // Copy current postion into the previous position
         _prevPos = curPos;
         // The new rotation needs to be applied after the previous
         // rotation, so premultiply:
         //
         // rotation = new_rotation * old_rotation
         //
         // If you use this class, replace this section with your own matrix
         // class. This section ugly, but cuts down on the amount of code in
         // this example and you probably already have your own matrix class.
         glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
         _trans = rotation * _trans;
      }
   }
}

//----------------------------------------------------------------------
// Start tracking motion
//----------------------------------------------------------------------
void Trackball::start(int x, int y)
{						
   _tracking = true;
   _prevPos = projection(x, y);
}

//----------------------------------------------------------------------
// Stop tracking motion
//----------------------------------------------------------------------
void Trackball::stop(void)
{
   _tracking = false;
}
