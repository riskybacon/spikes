#include <opengl.h>

#include <iostream>

#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <objc/objc.h>

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
: _text(NULL)
{
   initGL();

   _linearRGBColorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
   assert(_linearRGBColorspace != NULL);

   setText(text);
   setFont(font, pointSize);
   setForegroundColor(fgColor);
   setAlign(align);
   setLineSpacing(1.0f);
   update();
}

FontTexture::~FontTexture()
{
   freeGL();
}

/**
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

/**
 * Free OpenGL resources
 */
void FontTexture::freeGL()
{
   glDeleteTextures(1, &_id);
}

#ifdef __APPLE__

/*
 * Set the text to be rendered
 *
 * @param text
 *    The text to be rendered
 */
void FontTexture::setText(const std::string& text)
{
   _text = CFStringCreateWithCString(kCFAllocatorDefault,
                                     text.c_str(),
                                     kCFStringEncodingASCII);
}

/**
 * Set alignment
 */
void FontTexture::setAlign(TextAlign align)
{
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
}

/**
 * Set foreground color
 *
 * @param fgColor
 *    The color to use for the foreground color
 */
void FontTexture::setForegroundColor(const glm::vec4& fgColor)
{
   CGFloat colorDst[4];

   colorDst[0] = fgColor.r;
   colorDst[1] = fgColor.g;
   colorDst[2] = fgColor.b;
   colorDst[3] = fgColor.a;
   
   _fgColor = CGColorCreate(_linearRGBColorspace, colorDst);
}

/*
 * Create font for use in attributed string. Updates the _font member.
 *
 * @param fontNameSrc
 *    The name of the font family
 *
 * @param pointSize
 *    The size of the font in points
 */
void FontTexture::setFont(const std::string& fontNameSrc, float pointSize)
{
   CFStringRef fontNameDst = CFStringCreateWithCString(kCFAllocatorDefault,
                                                       fontNameSrc.c_str(),
                                                       kCFStringEncodingASCII);

   // Create the font with the specified name, point size, and an identity transformation matrix
   _font = CTFontCreateWithName(fontNameDst, (CGFloat)pointSize, NULL);
   assert(_font != NULL);
}

/*
 * Creates a bitmap context that contains the rendered string. Result stored in
 * private variable _ctx
 *
 * @return The CGContextRef that contains the bitmap data for the texture
 */
void FontTexture::createContext()
{
   _ctx = NULL;

   // Acquire a frame setter - this creates the set of lines needed to draw
   // the text string.
   CTFramesetterRef frameSetter = CTFramesetterCreateWithAttributedString(_attrString);
   
   if(frameSetter != NULL)
   {
      // Create a path for layout
      CGMutablePathRef path = CGPathCreateMutable();
      
      if(path != NULL)
      {
         CFRange fitRange;
         CGSize  constraint = CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX);
         
         // Get the CoreText suggested size from our framesetter
         CGSize frameSize = CTFramesetterSuggestFrameSizeWithConstraints(frameSetter,
                                                                         _attrRange,
                                                                         NULL,
                                                                         constraint,
                                                                         &fitRange);
         
         assert(frameSize.width > 0);
         assert(frameSize.height > 0);
         _texSize = glm::vec2(frameSize.width, frameSize.height);
         
         // Set path bounds
         CGRect bounds = CGRectMake(0, 0, frameSize.width, frameSize.height);
         
         // Bound the path
         CGPathAddRect(path, NULL, bounds);
         
         // Layout the attributed string in a frame
         CTFrameRef frame = CTFramesetterCreateFrame(frameSetter, fitRange, path, NULL);
         
         if(frame != NULL)
         {
            // Compute bounds for the bitmap context
            size_t width  = size_t(frameSize.width);
            size_t height = size_t(frameSize.height);
            size_t stride = sizeof(GLuint) * width;
            
            // Create bitmap context
            _ctx = CGBitmapContextCreate(NULL,
                                        width,
                                        height,
                                        8,
                                        stride,
                                        _linearRGBColorspace,
                                        kCGImageAlphaPremultipliedLast);
            
            if(_ctx != NULL)
            {
               // Use this for vertical reflection
               CGContextTranslateCTM(_ctx, 0.0, height);
               CGContextScaleCTM(_ctx, 1.0, -1.0);
               
               // Previously, a path was created using the attributed string,
               // the path was added to the frame. Now, draw the frame
               // into the bitmap context
               CTFrameDraw(frame, _ctx);
               
               // Flush the context
               CGContextFlush(_ctx);
            }
            else
            {
               std::cerr << "Expected context to be created" << std::endl;
               assert(0);
            }
            CFRelease(frame);
         }
         CFRelease(path);
      }
      CFRelease(frameSetter);
   }
}

/**
 * Create an attributed string
 */
void FontTexture::createAttributedString()
{
   _attrString = NULL;
	
	if(_text != NULL )
	{
      const CFIndex numParagraphSettings = 2;
      CTParagraphStyleSetting paragraphSettings[numParagraphSettings] =
      {
			{
            kCTParagraphStyleSpecifierAlignment,
				sizeof(_align),
				&_align

			},
			{
				kCTParagraphStyleSpecifierLineHeightMultiple,
				sizeof(CGFloat),
				&_lineSpacing,
         }
      };

      CTParagraphStyleRef paragraphStyle = CTParagraphStyleCreate(paragraphSettings, numParagraphSettings);

		if(paragraphStyle != NULL)
		{
         const CFIndex numAttrSettings = 3;
         
         CFStringRef keys[numAttrSettings] =
         {
            kCTParagraphStyleAttributeName,
            kCTFontAttributeName,
            kCTForegroundColorAttributeName,
         };
         
         CFTypeRef values[numAttrSettings] =
         {
            paragraphStyle,
            _font,
            _fgColor,
         };
         
         // Create a dictionary of attributes for our string
         CFDictionaryRef attr = CFDictionaryCreate(NULL,
                                                   (const void **)&keys,
                                                   (const void **)&values,
                                                   numAttrSettings,
                                                   &kCFTypeDictionaryKeyCallBacks,
                                                   &kCFTypeDictionaryValueCallBacks);

         if(attr != NULL)
         {
            // Creating a mutable attributed string
            _attrString = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
            
            if(_attrString != NULL)
            {
               // Set a mutable attributed string with the input string
               CFAttributedStringReplaceString(_attrString, CFRangeMake(0, 0), _text);
               
               // Compute the mutable attributed string range
               _attrRange = CFRangeMake(0, CFAttributedStringGetLength(_attrString));
               
               // Set the attributes
               CFAttributedStringSetAttributes(_attrString, _attrRange, attr, NO);
            }
         }
			CFRelease(paragraphStyle);
		}
	}
}


/*
 * Update the texture map
 */
void FontTexture::update()
{
   // Sets _attrString and _attrRange.
   createAttributedString();
   
   if(_attrString != NULL)
   {
      // Sets _ctx
      createContext();
   
      if(_ctx != NULL)
      {
         GLuint width  = GLuint(CGBitmapContextGetWidth(_ctx));
         GLuint height = GLuint(CGBitmapContextGetHeight(_ctx));
      
         _texSize = glm::vec2(width, height);
         
         const GLvoid* data = CGBitmapContextGetData(_ctx);

         // Bind a texture with ID
         glBindTexture(GL_TEXTURE_2D, _id);

         // Copy the context data to the GL
         glTexImage2D(GL_TEXTURE_2D,
                      0,
                      GL_RGBA,
                      width,
                      height,
                      0,
                      GL_RGBA,
                      GL_UNSIGNED_INT_8_8_8_8_REV,
                      data);
         CGContextRelease(_ctx);
      }
      CFRelease(_attrString);
   }
}
#endif

#ifdef __WINDOWS__
void TextureFont::setFont(const std::string& fontName, float pointSize)
{
   _fontName = fontName;
   _pointSize = pointSize;
   
   _font = CreateFont()
   
}
#endif