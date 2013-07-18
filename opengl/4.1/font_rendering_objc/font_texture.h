
#ifndef _FONT_TEXTURE_OSX_H
#define _FONT_TEXTURE_OSX_H

#include <opengl.h>
#include <string>
#include <glm/glm.hpp>

#ifdef __APPLE__
// Apple specific includes
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <objc/objc.h>
#endif

#ifdef __WINDOWS__
// Windows specific includes
#endif

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
      _lineSpacing = (CGFloat) spacing;
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

#ifdef __APPLE__
   //{ OS X Specific functions
   /**
    * Creates a bitmap context that contains the rendered string. Result stored in
    * private variable _ctx
    */
   void createContext();

   /**
    * Create an attributed string using existing text. Call setFont(), setLineSpacing() and setText()
    * prior to calling this method. Once those methods have been called, this method can be called
    * without repeated calls to the aforementioned methods.
    */
   void createAttributedString();
   //}
#endif

private:
   GLuint            _id;           //< Texture ID handle
   glm::vec2         _texSize;      //< Size of texture in texels

#ifdef __APPLE__
   //{ OS X specific
   CGFloat                      _lineSpacing;
   CTTextAlignment              _align;           //< Alignment to use (right, center, justified, etc)
   CTFontRef                    _font;            //< The font face and point size
   CFStringRef                  _text;            //< Text to be displayed
   CFMutableAttributedStringRef _attrString;      //< String with attributes.
   CGColorRef                   _fgColor;
   CGColorSpaceRef              _linearRGBColorspace;
   CFRange                      _attrRange;       //< Range of the string over which the attributes apply
   CGContextRef                 _ctx;
   //}
#endif
   
#ifdef __WINDOWS__
   HFONT _font;
#endif
};
#endif