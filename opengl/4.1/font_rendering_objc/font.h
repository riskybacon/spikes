#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

using std::vector;

class Font {
public:
  /**
   * Constructor
   *
   * @param filename
   *    Name of file that contains the TrueType font
   * @param pointSize
   *    Desired point size
   */
  Font(const std::string& filename, const int pointSize);
  
  /**
   * Destructor
   */
  ~Font();
  
  /**
   * Create a bitmap
   *
   * @param text
   *    The text to render into the bitmap
   */
  void createBitmap(const std::string& text);
  
  /**
   * Draw a glyph into the bitmap
   *
   * @param bitmap
   *    A pointer to a bitmap that contains the glyph to be added to the bitmap
   * @param x
   *    The x position in the destination bitmap
   * @param y
   *    The y position in the destination bitmap
   */
  void drawBitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y);
  
  /**
   * Load the set of glyphs needed to render a string and
   * compute the bounding box
   */
  void loadGlyphs(const std::string& text);
  
  /**
   * @return the texel width of the bitmap. This is the next
   *  power of 2 greater than the bounding box width
   */
  unsigned int const bitmapWidth(void) const {
    return mTexWidth;
  }
  
  /**
   * @return the texel height of the bitmap. This is the next
   *  power of 2 greater than the bounding box height
   */
  unsigned int const bitmapHeight(void) const {
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
  
  /**
   * @return pointer to the bitmap data
   */
  unsigned char* const data(void) const {
    return mData;
  }
  
private:
  const std::string      mFilename;     //< filename that contains the font
  const int              mPointSize;    //< Point size for this font
  FT_Library             mLibrary;      //< Font library object
  FT_Face                mFace;         //< Font face object
  std::vector<FT_Glyph>  mGlyphs;       //< Glyphs that make up a string
  std::vector<FT_Pos>    mXPos;         //< X position of glyphs in the string
  std::vector<FT_Pos>    mYMax;         //< yMax of each glyph in the string
  std::vector<FT_Pos>    mYShift;       //< Amount to shift in Y direction from 0
  unsigned int           mTexWidth;     //< The width of the texture. Always a power of 2
  unsigned int           mTexHeight;    //< The height of the texture. Always a power of 2
  unsigned int           mBBoxWidth;    //< Width of the string's bounding box within the bitmap
  unsigned int           mBBoxHeight;   //< Height of the string's bounding box within the bitmap
  FT_Bool                mUseKerning;   //< Does this font allow the use of kerning?
  unsigned char*         mData;         //< Bitmap data
};
