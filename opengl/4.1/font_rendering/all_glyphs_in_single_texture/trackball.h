//----------------------------------------------------------------------
// Virtual Trackball
//
// Author: Jeff Bowles <jbowles@riskybacon.com>
//----------------------------------------------------------------------

#ifndef _Trackball_h
#define _Trackball_h

// Dependencies: glm, 
#include <glm/glm.hpp>

/**
 *
 * Maps mouse movements to a virtual trackball. The mouse movements
 * are turned into a rotation matrix.
 *
 * How to use this class:
 * \code
 * // Create a new instance - multiple instances are possible
 * Trackball* _trackball = new Trackball(windowWidth, windowHeight)
 *
 * // After a window reshape event:
 * _trackball->reshape(windowWidth, windowHeight);
 *
 * // When mouse button is pressed:
 * _trackball->start(mouseX,mouseY);
 *
 * // When mouse is in motion:
 * _trackball->motion(mouseX, mouseY);
 *
 * // When mouse button is released:
 * _trackball->stop();
 *
 * \endcode
 */
class Trackball
{
public:
   ///
   /// Creates a new trackball instance.
   ///
   /// \param width The width of the window
   /// \param height The height of the window
   ///
   Trackball(int, int);
   
   ///
   /// Destroys a trackball instance
   ///
   virtual ~Trackball(void) {}
   
   ///
   /// Check to see if motion is being tracked.
   ///
   /// \return true if motion is being tracked. false if not
   ///
   const bool isTracking(void) const { return _tracking; }
   
   ///
   /// Project the window coordinates onto the trackball sphere
   ///
   /// \param x X position of the mouse
   /// \param y Y position of the mouse
   /// \param v The x,y,z projection
   ///
   glm::vec3 projection(int x, int y);
   
   ///
   /// Track mouse motion and update the transformation
   ///
   /// \param x X position of the mouse
   /// \param y Y position of the mouse
   ///
   void motion(int x, int y);
   
   ///
   /// Start tracking mouse motion
   ///
   /// \param x X position of the mouse
   /// \param y Y position of the mouse
   ///
   void start(int x, int y);
   
   ///
   /// Stop tracking mouse motion
   ///
   void stop(void);
   
   ///
   /// Reset the transformation to the identity matrix
   ///
   void reset(void);
   
   ///
   /// Change the size of the area that is being projected onto the 
   /// trackball sphere. Call this when the window size changes.
   ///
   ///
   /// \param width The width of the window
   /// \param height The height of the window
   ///
   void reshape(int, int);
   
   glm::mat4 const getTransform(void) const { return _trans; }
   
private:
   
   bool      _tracking;     //< True if motion being tracked
   bool      _havePrevPos;  //< True if there is a previous position, false if tracking just started
   glm::vec3 _prevPos;      //< Last recorded position of the trackball.
   glm::mat4 _trans;        //< Transformation matrix
   int       _width;        //< Window width
   int       _height;       //< Window height
};

#endif
