#include <opengl.h>

#include <iostream>

// OpenGL utilities header
#include "font_texture.h"


/**
 * Constructor. This should be called after OpenGL has been initialized. The text
 * will be rendered to a bitmap, and an OpenGL texture map will be created and initialized.
 *
 * @param font
 *    Name of the font to use
 *
 * @param text
 *    The text to render
 *
 * @param pointSize
 *    The point size of the font to use
 *
 * @param fgColor
 *    Foreground color of the text
 *
 * @param bgColor
 *    Background color of the text
 *
 * @param alignment
 *
 */
FontTexture::FontTexture(const std::string& font, const std::string& text, float pointSize, const glm::vec4& fgColor, TextAlign align)
: _fontName(font)
, _text(text)
, _pointSize(pointSize)
, _face(NULL)
, _data(NULL)
{
   initGL();
   initPlatform();

   setText(text);
   setFont(font, pointSize);
   setForegroundColor(fgColor);
   setAlign(align);
   setLineSpacing(1.0f);
   update();
}

/*
 * Destructor
 */
FontTexture::~FontTexture()
{
   freeGL();
   freePlatform();
}

/*
 * Platform specific initialization
 */
void FontTexture::initPlatform()
{
   // Initialize freetype library
   FT_ASSERT(FT_Init_FreeType(&_library));
   setFont(_fontName, _pointSize);
}

/*
 * Free platform specific resources
 */
void FontTexture::freePlatform()
{
   FT_Done_Face    ( _face );
   FT_Done_FreeType( _library );
   
   if(_data != NULL)
   {
      delete _data;
   }

}
/*
 * Initialize OpenGL resources
 */
void FontTexture::initGL()
{
   glGenTextures(1, &_id);
   
   glBindTexture(GL_TEXTURE_2D, _id);
   //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GL_ERR_CHECK();
}

/*
 * Free OpenGL resources
 */
void FontTexture::freeGL()
{
   glDeleteTextures(1, &_id);
}

/*
 * Set the text to be rendered
 *
 * @param text
 *    The text to be rendered
 */
void FontTexture::setText(const std::string& text)
{
   _text = text;
}

/**
 * Set alignment
 */
void FontTexture::setAlign(TextAlign align)
{
#if 0
   switch(align)
   {
      case TEXT_ALIGN_CENTER:
         _align = kCTTextAlignmentCenter;
         break;
         
      case TEXT_ALIGN_LEFT:
         _align = kCTTextAlignmentLeft;
         break;
         
      case TEXT_ALIGN_RIGHT:
         _align = kCTTextAlignmentRight;
         break;
         
      case TEXT_ALIGN_JUSTIFIED:
         _align = kCTTextAlignmentJustified;
         break;
         
      default:
         _align = kCTTextAlignmentLeft;
         break;
   }
#endif
}

/**
 * Set foreground color
 *
 * @param fgColor
 *    The color to use for the foreground color
 */
void FontTexture::setForegroundColor(const glm::vec4& fgColor)
{
}

/*
 * Create font for use in attributed string. Updates the _font member.
 *
 * @param fontName
 *    The name of the font family
 *
 * @param pointSize
 *    The size of the font in points
 */
void FontTexture::setFont(const std::string& fontName, float pointSize)
{
   _fontName = fontName;
   _pointSize = pointSize;

   if(_face != NULL)
   {
      FT_Done_Face(_face);
   }
   
   // Create face object
   FT_ASSERT(FT_New_Face(_library, _fontName.c_str(), 0, &_face));
   
   // Set the character size at 100dpi
   FT_ASSERT(FT_Set_Char_Size(_face, _pointSize * 64, 0, 100, 0));
   
   // Check to see if this font has kerning. If so, use it.
   _useKerning = FT_HAS_KERNING(_face);
}

/**
 * Draw a bitmap glyph into the bitmap that will
 * contain the string
 *
 * @param bitmap
 *    The bitmap that contains the glyph
 * @param x
 *    The starting x position in the texture
 * @param y
 *    The starting y position in the texture
 */
void FontTexture::drawBitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y)
{
   FT_Int  i, j, p, q;
   FT_Int  x_max = x + bitmap->width;
   FT_Int  y_max = y + bitmap->rows;
   
   for ( i = x, p = 0; i < x_max; i++, p++ )
   {
      for ( j = y, q = 0; j < y_max; j++, q++ )
      {
         if ( i < 0      || j < 0       ||
             i >= (FT_Int)_texWidth || j >= (FT_Int)_texHeight )
         {
            std::cout << "continuing" << std::endl;
            continue;
         }
         _data[j * _texWidth + i] |= bitmap->buffer[q * bitmap->width + p];
      }
   }
}

/**
 * Find the next power of 2
 */
int nextPowerOf2(int val) {
   val--;
   val = (val >> 1) | val;
   val = (val >> 2) | val;
   val = (val >> 4) | val;
   val = (val >> 8) | val;
   val = (val >> 16) | val;
   val++; // Val is now the next highest power of 2.
   return val;
}

/**
 * Load the set of glyphs needed to render a string and
 * compute the bounding box
 */
void FontTexture::loadGlyphs(const std::string& text)
{
   FT_GlyphSlot  slot = _face->glyph;   /* a small shortcut */
   FT_UInt       glyphIndex;
   FT_Matrix     matrix;
   FT_Vector     pen;
   FT_UInt       previous;
   FT_Error      error;
   _glyphs.resize(text.length());
   _xPos.resize(text.length());
   _yMax.resize(text.length());
   _yShift.resize(text.length());
   
   
   // Initialize pen
   pen.x = 0;
   pen.y = 0;
   
   // OpenGL texcoord (0,0) is in the lower left,
   // FreeType coords put (0,0) in upper left. Transform
   // for use in OpenGL. This should be turned into an
   // option so that this code is not tied to OpenGL
   matrix.xx = (FT_Fixed)( 1 * 0x10000L );
   matrix.xy = (FT_Fixed)( 0 * 0x10000L );
   matrix.yx = (FT_Fixed)( 0 * 0x10000L );
   matrix.yy = (FT_Fixed)(-1 * 0x10000L );
   FT_Set_Transform(_face, &matrix, &pen);
   
   pen.x = 0;
   pen.y = 0;
   
   previous    = 0;
   FT_Pos yMin;
   FT_Pos yMax;
   
   for ( size_t n = 0; n < text.length(); ++n )
   {
      // Convert character code to glyph index
      glyphIndex = FT_Get_Char_Index(_face, text[n]);
      // Retrieve kerning distance and move pen position
      if ( _useKerning && previous && glyphIndex )
      {
         FT_Vector  delta;
         
         
         FT_Get_Kerning( _face, previous, glyphIndex,
                        FT_KERNING_DEFAULT, &delta );
         
         pen.x += delta.x >> 6;
      }
      // store current pen position
      _xPos[n] = pen.x;
      
      // Load glyph image into slot without rendering
      error = FT_Load_Glyph(_face, glyphIndex, FT_LOAD_DEFAULT);
      if ( error )
         continue;  /* ignore errors, jump to next glyph */
      
      // Extract glyph image and store it in the list
      error = FT_Get_Glyph(_face->glyph, &_glyphs[n] );
      if ( error )
         continue;  /* ignore errors, jump to next glyph */
      
      // increment pen position
      pen.x += slot->advance.x >> 6;
      
      // Record current glyph index for kerning purposes
      previous = glyphIndex;
      
      // Get the bounding box for the glyph
      FT_BBox  bbox;
      FT_Glyph_Get_CBox(_glyphs[n], ft_glyph_bbox_pixels, &bbox );
      
      // Get the maximum y position from the baseline for this glyph
      _yMax[n] = bbox.yMax;
      
      // Find the smallest and largest Y offsets from the baseline.
      // this will be used to determine the size of the bitmap
      // needed to hold the rendered string
      if( n == 0 )
      {
         yMin = bbox.yMin;
         yMax = bbox.yMax;
      }
      else
      {
         yMin = bbox.yMin < yMin ? bbox.yMin : yMin;
         yMax = bbox.yMax > yMax ? bbox.yMax : yMax;
      }
   }
   
   //  std::cout << "pen.x, yMin, yMax: " << pen.x << ", " << yMin << ", " << yMax << std::endl;
   
   // Get the shifts needed to push a font down lower in the bitmap. Remember that (0,0)
   // in the bitmap is the upper left corner
   for( size_t n = 0; n < text.length(); ++n ) {
      _yShift[n] = yMax - _yMax[n];
   }
   
   // Get the height and width of the string's bounding boxx in the bitmap
   _bBoxHeight = yMax - yMin;
   _bBoxWidth = pen.x;
   
   _texWidth = nextPowerOf2(_bBoxWidth);
   _texHeight = nextPowerOf2(_bBoxHeight);
}

/*
 * Create a bitmap
 *
 * @param text
 *    The text to render into the bitmap
 */
void FontTexture::createBitmap(const std::string& text)
{
   FT_GlyphSlot  slot;
   FT_Matrix     matrix;                 /* transformation matrix */
   FT_Vector     pen;                    /* untransformed origin  */
   FT_Error      error;
   FT_UInt       glyphIndex;
   
   // Load glyphs, computing bounding box
   loadGlyphs(text);
   
   if(_data != NULL) {
      delete _data;
   }
   
   // Create the texture map
   _data = new unsigned char[_texWidth * _texHeight];
   // Initialize texture map to zero
   memset(_data, 0, _texWidth * _texHeight);
   
   slot = _face->glyph;
   
   // Initialize pen
   pen.x = 0;
   pen.y = 0;
   
   // OpenGL texcoord (0,0) is in the lower left,
   // FreeType coords put (0,0) in upper left. Transform
   // for use in OpenGL. This should be turned into an
   // option so that this code is not tied to OpenGL
   matrix.xx = (FT_Fixed)( 1 * 0x10000L );
   matrix.xy = (FT_Fixed)( 0 * 0x10000L );
   matrix.yx = (FT_Fixed)( 0 * 0x10000L );
   matrix.yy = (FT_Fixed)(-1 * 0x10000L );
   FT_Set_Transform(_face, &matrix, &pen);
   
   for ( size_t n = 0; n < text.length(); n++ )
   {
      // Convert character code to glyph index
      glyphIndex = FT_Get_Char_Index(_face, text[n]);
      
      // Load glyph image into the slot, render the glyph and erase previous one
      error = FT_Load_Glyph(_face, glyphIndex, FT_LOAD_RENDER);
      if ( error )
         continue;  // TODO: handle errors
      
      //    printOutlineInfo(text[n], &slot->outline);
      
      // load glyph image into the slot and erase the previous one
      error = FT_Load_Glyph(_face, glyphIndex, FT_LOAD_DEFAULT);
      if ( error )
         continue;  /* ignore errors */
      
      /* convert to an anti-aliased bitmap */
      error = FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_NORMAL);
      
      // Draw glyph into texture map
      // Notice how slot->bitmap_left is used to increase the x position? Why
      // not compute that number in loadGlyphs()? Because that number is 0 each
      // time in loadGlyphs(). I suspect this is the case because the glyph isn't
      // actually rendered in that function, and is rendered in this function.
      drawBitmap(&slot->bitmap, _xPos[n] + slot->bitmap_left, _yShift[n] );
   }
}

/*
 * Update the texture map
 */
void FontTexture::update()
{
   createBitmap(_text);
   glBindTexture(GL_TEXTURE_2D, _id);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _texWidth, _texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, _data);

   _texSize = glm::vec2((float)_texWidth, (float)_texHeight);
}