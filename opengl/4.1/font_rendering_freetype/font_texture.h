
#ifndef _FONT_TEXTURE_OSX_H
#define _FONT_TEXTURE_OSX_H

#include <opengl.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

enum TextAlign
{
   TEXT_ALIGN_LEFT,
   TEXT_ALIGN_CENTER,
   TEXT_ALIGN_RIGHT,
   TEXT_ALIGN_JUSTIFIED
};

/**
 * Draw text onto a texture map
 */
class FontTexture
{
public:

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
   FontTexture(const std::string& font, const std::string& text, float pointSize, const glm::vec4& fgColor, TextAlign align);

   /**
    * Destructor
    */
   ~FontTexture();
   
   /**
    * @return the OpenGL texture handle
    */
   GLuint getID() const
   {
      return _id;
   }
   

   /**
    * Set alignment
    * 
    * @param align
    *    The alignment
    */
   void setAlign(TextAlign align);

   /**
    * Set foreground color
    *
    * @param fgColor
    *    The color to use for the foreground color
    */
   void setForegroundColor(const glm::vec4& fgColor);

   /**
    * Create font for use in attributed string. Updates the _font member.
    *
    * @param fontNameSrc
    *    The name of the font family
    *
    * @param pointSize
    *    The size of the font in points
    */
   void setFont(const std::string& fontNameSrc, float pointSize);
   
   /**
    * Set the line spacing
    *
    * @param spacing
    *    The line spacing
    */
   void setLineSpacing(float spacing = 1.0f)
   {
      _lineSpacing = spacing;
   }

   /**
    * Set the text to be rendered
    *
    * @param text
    *    The text to be rendered
    */
   void setText(const std::string& text);
   
   /**
    * @return size of texture map
    */
   glm::vec2 getSize() const
   {
      return _texSize;
   }

   /**
    * Update the texture map
    */
   void update();
   
private:
   /**
    * Initialize OpenGL resources
    */
   void initGL();
   
   /**
    * Free OpenGL resources
    */
   void freeGL();

   /**
    * Platform specific initialization
    */
   void initPlatform();

   /**
    * Free platform specific resources
    */
   void freePlatform();
   
   
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

private:
   GLuint                 _id;           //< Texture ID handle
   glm::vec2              _texSize;      //< Size of texture in texels
   float                  _lineSpacing;
   std::string            _fontName;
   std::string            _text;
   glm::vec4              _fgColor;
   
   std::string            _filename;     //< filename that contains the font
   int                    _pointSize;    //< Point size for this font
   FT_Library             _library;      //< Font library object
   FT_Face                _face;         //< Font face object
   std::vector<FT_Glyph>  _glyphs;       //< Glyphs that make up a string
   std::vector<FT_Pos>    _xPos;         //< X position of glyphs in the string
   std::vector<FT_Pos>    _yMax;         //< yMax of each glyph in the string
   std::vector<FT_Pos>    _yShift;       //< Amount to shift in Y direction from 0
   unsigned int           _texWidth;     //< The width of the texture. Always a power of 2
   unsigned int           _texHeight;    //< The height of the texture. Always a power of 2
   unsigned int           _bBoxWidth;    //< Width of the string's bounding box within the bitmap
   unsigned int           _bBoxHeight;   //< Height of the string's bounding box within the bitmap
   FT_Bool                _useKerning;   //< Does this font allow the use of kerning?
   unsigned char*         _data;         //< Bitmap data


};


#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };

const struct
{
   int          err_code;
   const char*  err_msg;
} ft_errors[] =

#include FT_ERRORS_H

#include <sstream>
#include <stdexcept>

extern "C"
{
   // This function does nothing. Called when FREETYPE_ASSERT fails and is about to
   // throw an exception. Put your breakpoint here.
   inline void ft_assert_breakpoint() {}
}


#ifdef __GNUC__
// There is a bug in version 4.4.5 of GCC on Ubuntu which causes GCC to segfault
// when __PRETTY_FUNCTION__ is used within certain templated functions.
#  if !(__GNUC__ == 4 && __GNUC_MINOR__ == 4 && __GNUC_PATCHLEVEL__ == 5)
#    define FT_FUNCTION_NAME __PRETTY_FUNCTION__
#  else
#    define FT_FUNCTION_NAME "unknown function"
#  endif
#elif _MSC_VER
#define FT_FUNCTION_NAME __FUNCSIG__
#else
#define FT_FUNCTION_NAME "unknown function"
#endif

#define FT_ASSERT(_exp) \
{ \
   FT_Error ft_err = _exp; \
   if(ft_err != 0) \
   { \
      assert_breakpoint(); \
      std::ostringstream ftassert__out; \
      ftassert__out << "Error in file " << __FILE__ << ":" << __LINE__ << std::endl; \
      ftassert__out << FT_FUNCTION_NAME << std::endl << std::endl; \
      ftassert__out << "Failed expression: " << #_exp << "." << std::endl;       \
      ftassert__out << std::boolalpha << ft_errors[ft_err].err_msg << std::endl; \
      throw std::runtime_error(ftassert__out.str());                                         \
   } \
}

#endif