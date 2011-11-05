#ifndef _Font_h
#define _Font_h

// FreeType headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

// STL headers
#include <vector>
#include <string>

// Exceptions
#include <stdexcept>

/**
 * Creates bitmap version of a TrueType font, suitable for
 * use in OpenGL. While this class was designed for use with
 * OpenGL, there are no OpenGL dependencies.
 *
 * A large texture is created with the first 128 glyphs in the bitmap.
 * The glyphs are laid out on a grid and the texture coordinates
 * for a particular glyph can be found using the texCoords method
 * and the size of the glyph can be found using the glyphSize method.
 */
class Font {
public:
  /**
   * Constructor
   * @param filename
   *  Name of file that contains the font
   * @param height
   *  Height of font in points
   */
  Font(const std::string& filename, float height);

  /**
   * Destructor
   */
  ~Font();
  
  /**
   * Get texture coordinates for a specific letter in the texture map
   */
  void texCoords(unsigned char ch, float& xMin, float& xMax, float& yMin, float& yMax);

  /**
   * @param ch
   *  A glyph
   * @return width for a glyph. Range is from (0,1)
   */
  float glyphWidth(unsigned char ch) {
    return mFontWidth[ch] / float(mGlyphHeight);
  }

  /**
   * @param ch
   *  A glyph
   * @return width for a glyph. Range is from (0,1)
   */
  float glyphHeight(unsigned char ch) {
    return mFontHeight[ch] / float(mGlyphWidth);
  }

  /**
   * @param ch
   *  A glyph
   * @return the aspect ratio (width : height) of the glyph
   */
  float glyphAspectRatio(unsigned char ch) {
    return float(mFontWidth[ch]) / float(mFontHeight[ch]);
  }
  
  /**
   * @return pointer to the data. The data is in floating point format
   *  with each data element being in the range from (0,1). There is
   */
  float* const data(void) const {
    return mData;
  }
  
  /**
   * @return width of the entire texture
   */
  int const texWidth(void) const {
    return mTexWidth;
  }

  /**
   * @return height of the entire texture
   */
  int const texHeight(void) const {
    return mTexHeight;
  }

protected:

  /**
   * Create bitmaps for each glyph
   */
  void createBitmap(const FT_Face&);

  /** 
   * Initialize the freetype library
   */
  void init(void);
  
private:
  std::string         mFilename;      //< Name of the font file
  float*              mData;          //< Floating point data
  int                 mNumGlyphs;     //< Number of glyphs
  float               mHeight;        //< Height of the font
  std::vector<float>  mFontWidth;     //< Width for each texture
  std::vector<float>  mFontHeight;    //< Height for each texture
  int                 mGlyphWidth;    //< Maximum width of a glyph
  int                 mGlyphHeight;   //< Maximum height of a glyph
  int                 mTexWidth;      //< Width of the texture
  int                 mTexHeight;     //< Height of the texture
  int                 mMaxRow;        //< Number of rows in the grid
  int                 mMaxCol;        //< Number of columns in the grid
};
#endif

