
#ifndef _FONT_TEXTURE_OSX_H
#define _FONT_TEXTURE_OSX_H

#include <opengl.h>
#include <string>
#include <glm/glm.hpp>
#include "font_texture.h"

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <objc/objc.h>

/**
 * OS X specific implementation for the Font Texture class
 */
class FontTextureOSX
{
public:

   /**
    * Constructor. This should be called after OpenGL has been initialized. The text
    * will be rendered to a bitmap, and an OpenGL texture map will be created and initialized.
    *
    * @param id
    *    OpenGL texture handle
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
   FontTextureOSX(GLuint id, const std::string& font, const std::string& text, float pointSize, const glm::vec4& fgColor, TextAlign align);

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
   //{ OS X Specific functions

   /**
    * @return The CGContextRef that contains the bitmap data for the texture
    */
   //   CGContextRef createContext(CFMutableAttributedStringRef attributedString);
   void createContext();

   /**
    * Set the attributed string properties. This method should only be called
    * after setFont(), setParagraphStyle() and setForegroundColor() have been called.
    * The method should also be called after any subsequent calls to those methods.
    */
   void setAttributedStringProperties();

   /**
    * Create an attributed string. Call setFont(), setLineSpacing() and setAttributedStringProperties()
    * prior to calling this method. Once those methods have been called, this method can be called
    * without repeated calls to the aforementioned methods.
    *
    * @param text
    *    The text to render
    */
   void createAttributedString(const std::string& text);
   
   /**
    * Create an attributed string using existing text. Call setFont(), setLineSpacing() and setAttributedStringProperties()
    * prior to calling this method. Once those methods have been called, this method can be called
    * without repeated calls to the aforementioned methods.
    */
   void createAttributedString();
   
   void createAttributedString(CFStringRef pString,
                               CTFontRef font,
                                                       CGColorRef pForegroundColor,
                                                       const CTTextAlignment nAlignment,
                                                       CFRange *pRange);

   /**
    * @return a CTParagraphStyleRef structure. NULL if an error occurred during creation.
    */
   CTParagraphStyleRef createParagraphStyle() const;

   /**
    * Create attributes for use in an attributed string
    *
    * @param paragraphStyle
    *    The paragraph style to use for this string
    *
    * @param font
    *    The font to use for this string
    *
    * @param fgColor
    *    The foreground color to use for this string
    *
    * @return attributes for use in a attributed string
    */
   CFDictionaryRef createAttributes(CTParagraphStyleRef paragraphStyle, CTFontRef font, CGColorRef fgColor) const;

   //CFMutableAttributedStringRef createAttributedString();

private:
   GLuint          _id;           //< Texture ID handle
   glm:: vec2       _texSize;      //< Size of texture in texels
   float           _lineSpacing;
   //{ OS X specific
   
   CTTextAlignment              _align;           //< Alignment to use (right, center, justified, etc)
   CTFontRef                    _font;            //< The font face and point size
   CFStringRef                  _text;            //< Text to be displayed
   CTParagraphStyleRef          _paragraphStyle;  //< Paragraph style (spacing, etc)
   CFDictionaryRef              _attributes;      //< Attributed string properties
   CFMutableAttributedStringRef _attrString;      //< String with attributes.
   CGColorRef                   _fgColor;
   CGColorSpaceRef              _linearRGBColorspace;
   CFRange                      _attrRange;       //< Range of the string over which the attributes apply
   CGContextRef                 _ctx;
   //}
};


void GLTexture2DString(GLuint texID,
                       const std::string& font,
                       const std::string& text,
                       const float nFontSize,
                       TextAlign nAlignment,
                       const glm::vec4& color,
                       glm::vec2& size
                      );

void GLTexture2DStringFontRef(GLuint      texID,
                       CTFontRef font,
                       CFStringRef text,
                       const CTTextAlignment nAlignment,
                       CGColorRef fgColor,
                       glm::vec2& size
                       );

void GLTexture2DAttrString(GLuint texID, CFMutableAttributedStringRef attrString, CFRange attrRange, glm::vec2& size);

void GLTexture2DCreateFromStringFontRef(GLuint texID,
                                        CTFontRef font,
                                        CFStringRef text,
                                        const CTTextAlignment alignment,
                                        CGColorRef fgColor,
                                        CGSize &rSize);

CFMutableAttributedStringRef CFMutableAttributedStringCreate(CFStringRef pString,
                                                             CTFontRef font,
                                                             CGColorRef pForegroundColor,
                                                             const CTTextAlignment nAlignment,
                                                             CFRange *pRange);

#if 0
// Generate a texture from a c-string, using a font, at a size,
// with an alignment and a color
GLuint GLTexture2DCreateFromString(const GLchar * const pString,
								   const GLchar * const pFontName,
								   const CGFloat nFontSize,
								   const CTTextAlignment nAlignment,
								   const CGFloat * const pColor,
								   NSSize &rSize);

// Generate a texture from a stl string, using a font, at a size,
// with an alignment and a color
GLuint GLTexture2DCreateFromString(const GLstring &rString,
								   const GLstring &rFontName,
								   const CGFloat nFontSize,
								   const CTTextAlignment nAlignment,
								   const CGFloat * const pColor,
								   NSSize &rSize);

// Generate a texture from a core foundation string, using a font,
// at a size, with an alignment and a color
GLuint GLTexture2DCreateFromString(CFStringRef pString,
								   CFStringRef pFontName,
								   const CGFloat nFontSize,
								   const CTTextAlignment nAlignment,
								   const CGFloat * const pColor,
								   NSSize &rSize);

// Generate a texture from a core foundation attributed string
GLuint GLTexture2DCreateFromString(CFAttributedStringRef pAttrString,
								   const CFRange &rRange,
								   NSSize &rSize);
#endif

#endif