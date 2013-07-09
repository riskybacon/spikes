#include "Font.h"

/**
 * Constructor
 *
 * @param filename
 *    Name of file that contains the TrueType font
 * @param pointSize
 *    Desired point size
 */
Font::Font(const std::string& filename, const int pointSize)
:  mFilename   (filename)
,  mPointSize  (pointSize)
,  mData       (NULL)
{
  FT_Error error;
  
  // Initialize freetype library
  error = FT_Init_FreeType( &mLibrary );
  
  // Create face object
  error = FT_New_Face( mLibrary, mFilename.c_str(), 0, &mFace );
  
  // Set the character size at 100dpi
  error = FT_Set_Char_Size( mFace, mPointSize * 64, 0,
                           100, 0 );   

  // Check to see if this font has kerning. If so, use it.
  mUseKerning = FT_HAS_KERNING( mFace );
}

/**
 * Destructor
 */
Font::~Font() {
  FT_Done_Face    ( mFace );
  FT_Done_FreeType( mLibrary );
  
  if(mData != NULL) {
    delete mData;
  }
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
void Font::drawBitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y) {
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;
  
  for ( i = x, p = 0; i < x_max; i++, p++ ) {
    for ( j = y, q = 0; j < y_max; j++, q++ ) {
      if ( i < 0      || j < 0       ||
          i >= (FT_Int)mTexWidth || j >= (FT_Int)mTexHeight ) {
        std::cout << "continuing" << std::endl;  
        continue; 
      }
      mData[j * mTexWidth + i] |= bitmap->buffer[q * bitmap->width + p];
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
void Font::loadGlyphs(const std::string& text) {
  FT_GlyphSlot  slot = mFace->glyph;   /* a small shortcut */
  FT_UInt       glyphIndex;
  FT_Matrix     matrix;
  FT_Vector     pen;
  FT_UInt       previous;
  FT_Error      error;
  mGlyphs.resize(text.length());
  mXPos.resize(text.length());
  mYMax.resize(text.length());
  mYShift.resize(text.length());
  
  
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
  FT_Set_Transform( mFace, &matrix, &pen );
  
  pen.x = 0; 
  pen.y = 0;
  
  previous    = 0;
  FT_Pos yMin;
  FT_Pos yMax;
  
  for ( size_t n = 0; n < text.length(); ++n )
  {
    // Convert character code to glyph index
    glyphIndex = FT_Get_Char_Index( mFace, text[n] );
    // Retrieve kerning distance and move pen position
    if ( mUseKerning && previous && glyphIndex )
    {
      FT_Vector  delta;
      
      
      FT_Get_Kerning( mFace, previous, glyphIndex,
                     FT_KERNING_DEFAULT, &delta );
      
      pen.x += delta.x >> 6;
    }
    // store current pen position
    mXPos[n] = pen.x;
    
    // Load glyph image into slot without rendering
    error = FT_Load_Glyph( mFace, glyphIndex, FT_LOAD_DEFAULT );
    if ( error )
      continue;  /* ignore errors, jump to next glyph */
    
    // Extract glyph image and store it in the list
    error = FT_Get_Glyph( mFace->glyph, &mGlyphs[n] );
    if ( error )
      continue;  /* ignore errors, jump to next glyph */
    
    // increment pen position
    pen.x += slot->advance.x >> 6;
    
    // Record current glyph index for kerning purposes
    previous = glyphIndex;
    
    // Get the bounding box for the glyph
    FT_BBox  bbox;
    FT_Glyph_Get_CBox( mGlyphs[n], ft_glyph_bbox_pixels, &bbox );
    
    // Get the maximum y position from the baseline for this glyph
    mYMax[n] = bbox.yMax;
    
    // Find the smallest and largest Y offsets from the baseline.
    // this will be used to determine the size of the bitmap
    // needed to hold the rendered string
    if( n == 0 ) {
      yMin = bbox.yMin;
      yMax = bbox.yMax;
    } else {
      yMin = bbox.yMin < yMin ? bbox.yMin : yMin;
      yMax = bbox.yMax > yMax ? bbox.yMax : yMax;
    }
  }
  
//  std::cout << "pen.x, yMin, yMax: " << pen.x << ", " << yMin << ", " << yMax << std::endl;
  
  // Get the shifts needed to push a font down lower in the bitmap. Remember that (0,0)
  // in the bitmap is the upper left corner
  for( size_t n = 0; n < text.length(); ++n ) {
    mYShift[n] = yMax - mYMax[n];
  }
  
  // Get the height and width of the string's bounding boxx in the bitmap
  mBBoxHeight = yMax - yMin;
  mBBoxWidth = pen.x;
  
  mTexWidth = nextPowerOf2(mBBoxWidth);
  mTexHeight = nextPowerOf2(mBBoxHeight);
  
}

void printTags(char tags)
{
   if(tags & 000000001) // Bezier control point?
   {
      std::cout << " bezier control point";
      if(tags & 00000010) 
      {
         std::cout << ", third order";
      }
      else
      {
         std::cout << ", second order";
      }
   }
   else if((tags & 0) == 0)
   {
      std::cout << " on curve";
   }
}

void printOutlineInfo(char text, FT_Outline* outline) 
{
   if(text != 'a') return;
   
   std::cout << text << ":" << std::endl;
   std::cout << "Num contours: " << outline->n_contours << std::endl;
   std::cout << "Num points: " << outline->n_points << std::endl;
   for( size_t i = 0 ; i < outline->n_points; ++i )
   {
      std::cout << "points[" << i << "]: " << outline->points[i].x << "," << outline->points[i].y;
      printTags(outline->tags[i]);
      std:: cout << std::endl;
   }
   
   for( size_t i = 0 ; i < outline->n_contours; ++i )
   {
      std::cout << "contour_end_points[" << i << "]: " << outline->contours[i] << std::endl;
   }
  
 
}
/**
 * Create a bitmap
 *
 * @param text
 *    The text to render into the bitmap
 */
void Font::createBitmap(const std::string& text) {
  FT_GlyphSlot  slot;
  FT_Matrix     matrix;                 /* transformation matrix */
  FT_Vector     pen;                    /* untransformed origin  */
  FT_Error      error;
  FT_UInt       glyphIndex;
  
  // Load glyphs, computing bounding box
  loadGlyphs(text);
  
  if(mData != NULL) {
    delete mData;
  }
  
  // Create the texture map
  mData = new unsigned char[mTexWidth * mTexHeight];
  std::cout << "size: " << sizeof(unsigned) * mTexWidth * mTexHeight << std::endl;
  // Initialize texture map to zero
  memset(mData, 0, mTexWidth * mTexHeight);
  
  slot = mFace->glyph;
  
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
  FT_Set_Transform( mFace, &matrix, &pen );
  
  for ( size_t n = 0; n < text.length(); n++ )
  {
    // Convert character code to glyph index
    glyphIndex = FT_Get_Char_Index( mFace, text[n] );
    
    // Load glyph image into the slot, render the glyph and erase previous one
    error = FT_Load_Glyph( mFace, glyphIndex, FT_LOAD_RENDER);
    if ( error )
      continue;  // TODO: handle errors

//    printOutlineInfo(text[n], &slot->outline);

    // load glyph image into the slot and erase the previous one
    error = FT_Load_Glyph( mFace, glyphIndex, FT_LOAD_DEFAULT );
    if ( error )
      continue;  /* ignore errors */
    
    /* convert to an anti-aliased bitmap */
    error = FT_Render_Glyph( mFace->glyph, FT_RENDER_MODE_NORMAL );
    
    // Draw glyph into texture map
    // Notice how slot->bitmap_left is used to increase the x position? Why
    // not compute that number in loadGlyphs()? Because that number is 0 each
    // time in loadGlyphs(). I suspect this is the case because the glyph isn't
    // actually rendered in that function, and is rendered in this function.
    drawBitmap( &slot->bitmap, mXPos[n] + slot->bitmap_left, mYShift[n] );
  }
}


