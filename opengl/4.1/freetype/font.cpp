#include <iostream>
#include "font.h"
#include "oglwrapper.h"

Font::Font(const std::string& filename, float height) 
: mFilename   (filename)
, mData       (NULL)
, mNumGlyphs  (128)
, mHeight     (height)
, mMaxRow     (12)
, mMaxCol     (12)
{
  mFontWidth.resize(mNumGlyphs);
  mFontHeight.resize(mNumGlyphs);
  init();
}

/**
 * Destructor
 */
Font::~Font() {
  if(mData != NULL) {
    delete mData;
  }
}

/**
 * Initialize the font
 */
void Font::init(void) {
	// Initialize a freetype font library
	FT_Library library;
	if(FT_Init_FreeType(&library)) {
		throw std::runtime_error("FT_Init_FreeType failed");
  }
  
	//The object in which Freetype holds information on a given
	//font is called a "face".
  // Create 
	FT_Face face;
  
  // Load a font from a file
	if(FT_New_Face(library, mFilename.c_str(), 0, &face)) {
    std::string error = std::string("Failed to load font from file ") + mFilename;
		throw std::runtime_error(error);
  }
  
  // Font sizes in freetype are measured in 1/64ths of a point,
  // so multiple the desired point size by 64.
	FT_Set_Char_Size(face, mHeight * 64, mHeight * 64, 96, 96);

  // Create a 2D texture map with all of the characters from the font
  createBitmap(face);

  // Clean up the face
	FT_Done_Face(face);
  
	// Clean up the library
	FT_Done_FreeType(library);
}

void Font::texCoords(unsigned char ch, float& xMin, float& xMax, float& yMin, float& yMax) {
  int col = ch % mMaxCol;
  int row = ch / mMaxRow;

  xMin = (col       * mGlyphWidth);
  xMax = (xMin + mFontWidth[ch] - 1);
  yMin = (row       * mGlyphHeight);
  yMax = (yMin + mFontHeight[ch] - 1);
  
  xMin /= mTexWidth;
  xMax /= mTexWidth;
  yMin /= mTexHeight;
  yMax /= mTexHeight;
}

void Font::createBitmap(const FT_Face& face) {
//  int mGlyphWidth;
//  int mGlyphHeight;
  
  // Find width and heights of the glyphs
  for(unsigned char ch = 0; ch < mNumGlyphs; ++ch) {
    // Load the Glyph for the requested character
    if(FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT)) {
      throw std::runtime_error("FT_Load_Glyph failed");
    }
    
    // Move the face's glyph into a Glyph object.
    FT_Glyph glyph;
    if(FT_Get_Glyph(face->glyph, &glyph)) {
      throw std::runtime_error("FT_Get_Glyph failed");
    }
    
    // Convert the glyph to a bitmap.
    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    //This reference will make accessing the bitmap easier
    FT_Bitmap& bitmap=bitmap_glyph->bitmap;

    int width  = bitmap.width;
    int height = bitmap.rows;
    
    mFontWidth[ch] = width;
    mFontHeight[ch] = height;
    
    if(ch == 0) {
      mGlyphWidth = width;
      mGlyphHeight = height;
    } else {
      mGlyphWidth  = width  > mGlyphWidth  ? width  : mGlyphWidth;
      mGlyphHeight = height > mGlyphHeight ? height : mGlyphHeight;
    }
  }
  

  mTexWidth = mMaxRow * mGlyphWidth;
  mTexHeight = mMaxCol * mGlyphHeight;

  mData = new float[mTexWidth * mTexHeight];
  
  // Fill in the data with zeros
  for(int i = 0; i < mTexWidth * mTexHeight; ++i) {
    mData[i] = 0.1f;
  }

  for(unsigned char ch = 0; ch < mNumGlyphs; ++ch) {
    // Load the Glyph for the requested character
    if(FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT))
      throw std::runtime_error("FT_Load_Glyph failed");
    
    // Move the face's glyph into a Glyph object.
    FT_Glyph glyph;
    if(FT_Get_Glyph(face->glyph, &glyph)) {
      throw std::runtime_error("FT_Get_Glyph failed");
    }
    
    // Convert the glyph to a bitmap.
    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    
    //This reference will make accessing the bitmap easier
    FT_Bitmap& bitmap=bitmap_glyph->bitmap;

    int col = ch % mMaxCol;
    int row = ch / mMaxRow;

    int xStart = (row * mTexWidth * mGlyphHeight) + col * mGlyphWidth;
    
    // Copy the bitmap into the texture data
    for(int v = 0; v < bitmap.rows; ++v) {
      for(int u = 0; u < bitmap.width; ++u) {
        int idx = xStart + v * mTexWidth + u;
        float color = bitmap.buffer[(bitmap.rows - v) * bitmap.width + u] / 255.0f;
        mData[idx] = color;
      }
    }
  }
}