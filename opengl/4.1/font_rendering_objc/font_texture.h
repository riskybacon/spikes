#pragma once

#include "font.h"
#include <shader.h>

/**
 * Creates a texture map for a string
 * of characters
 */
class FontTexture {
public:
  /**
   * Constructor
   * @param font
   *   Pointer to the font object used to render the text
   * @param text
   *   The string to render
   */
  FontTexture(Font* font, const std::string& text);

  /**
   * OpenGL specific initialization
   */
  void initGL(void);
  
  /**
   * Get the texture ID
   */
  GLuint texID(void) const {
    return mTexID;
  }

  /**
   * @return the texel width of the bitmap. This is the next
   *  power of 2 greater than the bounding box width
   */
  unsigned int const textureWidth(void) const {
    return mTexWidth;
  }
  
  /**
   * @return the texel height of the bitmap. This is the next
   *  power of 2 greater than the bounding box height
   */
  unsigned int const textureHeight(void) const {
    return mTexHeight;
  }
  
  /**
   * @return the texel height of the bounding box for the
   *  rendered font string. The bitmap height is the
   *  next power of 2 greater than this number.
   */
  unsigned int const boundingBoxHeight(void) const {
    return mBBoxHeight;
  }
  
  /**
   * @return the texel width of the bounding box for the
   *  rendered font string. The bitmap width is the
   *  next power of 2 greater than this number.
   */
  unsigned int const boundingBoxWidth(void) const {
    return mBBoxWidth;
  }
  
#if 0
  /** 
   * @return the maximum S texture coordinate. This
   *  is very similar to boundingBoxWidth(), except
   *  the value is in the (0,1) range, where a 1 would 
   *  be the same as bitmapWidth()
   */
  float const maxTexCoordS(void) const {
    return mMaxTexCoordS;
  }
  
  /** 
   * @return the maximum T texture coordinate. This
   *  is very similar to boundingBoxHeight(), except
   *  the value is in the (0,1) range, where a 1 would 
   *  be the same as bitmapHeight()
   */
  float const maxTexCoordT(void) const {
    return mMaxTexCoordT;
  }
  
  /**
   * @return the width of the quad needed to render this texture.
   *   This should probably be up higher in the code
   */ 
  float const quadWidth(void) const {
    return mQuadWidth;
  }

  /**
   * @return the height of the quad needed to render this texture.
   *   This should probably be up higher in the code
   */ 
  float const quadHeight(void) const {
    return mQuadHeight;
  }
#endif
  
private:
  GLuint       mTexID;        //< OpenGL texture map handle
  Font*        mFont;         //< Pointer to a font object for creating the texture map
  std::string  mText;         //< Text to render into the texture map
  unsigned int mBBoxWidth;    //< Bounding box width in texels
  unsigned int mBBoxHeight;   //< Bounding box height in texels
  unsigned int mTexWidth;     //< Texture width
  unsigned int mTexHeight;    //< Texture height
#if 0
  float        mQuadWidth;    //< Quad width
  float        mQuadHeight;   //< Quad height
  float        mMaxTexCoordS; //< Maximum S texture coord to use when rendering
  float        mMaxTexCoordT; //< Maximum S texture coord to use when rendering
#endif
};
