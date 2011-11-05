#ifndef _Font_h
#define _Font_h

// FreeType headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

// OpenGL headers 
#include "platform_specific.h"
#include <GL/glfw.h>

// STL headers
#include <vector>
#include <string>

// Exceptions
#include <stdexcept>

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

  void draw(void);

  GLuint texID(GLuint ch) {
#if 0
    return mTextures[ch];
#else
    return mTexID;
#endif
  }

  void texCoords(unsigned char ch, float& xMin, float& xMax, float& yMin, float& yMax);
  
  
protected:

  /**
   * Create bitmaps for each glyph
   */
  void createBitmapVector(const FT_Face&);

  /** 
   * Initialize the freetype library
   */
  void init(void);
  
  /**
   * Create texture maps for the font
   */
  void makeTexture(const FT_Face& face, unsigned char ch);
  
private:
  std::string         mFilename;      //< Name of the font file
  int                 mNumGlyphs;     //< Number of glyphs
  float               mHeight;        //< Height of the font
  GLuint              mTextures[128]; //< List of texture ids that correspond to glyphs
  std::vector<float>  mFontWidth;     //< Width for each texture
  std::vector<float>  mFontHeight;    //< Height for each texture
  int                 mGlyphWidth;    //< Maximum width of a glyph
  int                 mGlyphHeight;   //< Maximum height of a glyph
  int                 mTexWidth;      //< Width of the texture
  int                 mTexHeight;     //< Height of the texture
  GLuint              mTexID;         //< Font character
  int                 mMaxRow;
  int                 mMaxCol;
};
#endif

