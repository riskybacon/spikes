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
, _ctx(NULL)
{
   initGL();
   //   GLTexture2DString(id, font, text, pointSize, align, fgColor, _texSize);

   _linearRGBColorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
   assert(_linearRGBColorspace != NULL);

   setText(text);
   setFont(font, pointSize);
   setForegroundColor(fgColor);
   setAlign(align);
   setLineSpacing(1.0f);
   createAttributedString();
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
 * Creates a bitmap context that contains the rendered string. 
 *
 * @return The CGContextRef that contains the bitmap data for the texture
 */
void FontTexture::createContext()
{
   if(_ctx != NULL)
   {
      _ctx = NULL;
   }
   

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
            
            // Release the frame
            CFRelease(frame);
         } // if
         
         CFRelease(path);
      } // if
      
      CFRelease(frameSetter);
   } // if
}

/**
 * Create an attributed string
 */
void FontTexture::createAttributedString()
{
   if(_attrString != NULL)
   {
      CFRelease(_attrString);
   }

   _attrString = NULL;
	
	if( _text != NULL )
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
   createAttributedString();
   
   if(_attrString != NULL)
   {
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
   }
}

#if 0
// Create an attributed string from a CF string, font, justification, and font size
CFMutableAttributedStringRef CFMutableAttributedStringCreate(CFStringRef pString,
                                                                    CTFontRef font,
                                                                    CGColorRef pForegroundColor,
                                                                    const CTTextAlignment nAlignment,
                                                                    CFRange *pRange)
{
	CFMutableAttributedStringRef pAttrString = NULL;
	
	if( pString != NULL )
	{
		// Paragraph style setting structure
		const GLuint nCntStyle = 2;
		
		// For single spacing between the lines
		const CGFloat nLineHeightMultiple = 1.0f;
		
		// Paragraph settings with alignment and style
		CTParagraphStyleSetting settings[nCntStyle] =
		{
			{
				kCTParagraphStyleSpecifierAlignment,
				sizeof(nAlignment),
				&nAlignment
			},
			{
				kCTParagraphStyleSpecifierLineHeightMultiple,
				sizeof(CGFloat),
				&nLineHeightMultiple
			}
		};
		
		// Create a paragraph style
		CTParagraphStyleRef pStyle = CTParagraphStyleCreate(settings, nCntStyle);
		
		if( pStyle != NULL )
		{
         // Set attributed string properties
         const GLuint nCntDict = 3;
         
         CFStringRef keys[nCntDict] =
         {
            kCTParagraphStyleAttributeName,
            kCTFontAttributeName,
            kCTForegroundColorAttributeName,
         };
         
         CFTypeRef values[nCntDict] =
         {
            pStyle,
            font,
            pForegroundColor
         };
         
         // Create a dictionary of attributes for our string
         CFDictionaryRef pAttributes = CFDictionaryCreate(NULL,
                                                          (const void **)&keys,
                                                          (const void **)&values,
                                                          nCntDict,
                                                          &kCFTypeDictionaryKeyCallBacks,
                                                          &kCFTypeDictionaryValueCallBacks);
         
         if( pAttributes != NULL )
         {
            // Creating a mutable attributed string
            pAttrString = CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);
            
            if( pAttrString != NULL )
            {
               // Set a mutable attributed string with the input string
               CFAttributedStringReplaceString(pAttrString, CFRangeMake(0, 0), pString);
               
               // Compute the mutable attributed string range
               *pRange = CFRangeMake(0, CFAttributedStringGetLength(pAttrString));
               
               // Set the attributes
               CFAttributedStringSetAttributes(pAttrString, *pRange, pAttributes, NO);
            } // if
            
            CFRelease(pAttributes);
         } // if
         
			CFRelease(pStyle);
		} // if
	} // if
   
   return pAttrString;
} // CFMutableAttributedStringCreate

#pragma mark -
#pragma mark Private - Utilities - CG

// Create a bitmap context from a string, font, justification, and font size
static CGContextRef CGContextCreateFromAttributedString(CFAttributedStringRef pAttrString,
                                                        const CFRange &rRange,
                                                        CGColorSpaceRef pColorspace,
                                                        CGSize &rSize)
{
	CGContextRef pContext = NULL;
	
	if( pAttrString != NULL )
	{
		// Acquire a frame setter - this creates the set of lines needed to draw
      // the text string.
		CTFramesetterRef pFrameSetter = CTFramesetterCreateWithAttributedString(pAttrString);
		
		if( pFrameSetter != NULL )
		{
			// Create a path for layout
			CGMutablePathRef pPath = CGPathCreateMutable();
			
			if( pPath != NULL )
			{
				CFRange range;
				CGSize  constraint = CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX);
				
				// Get the CoreText suggested size from our framesetter
				rSize = CTFramesetterSuggestFrameSizeWithConstraints(pFrameSetter,
																	 rRange,
																	 NULL,
																	 constraint,
																	 &range);
				
            //std::cout << "rSize: " << rSize.width << "," << rSize.height << std::endl;
            
				// Set path bounds
				CGRect bounds = CGRectMake(0.0f,
										   0.0f,
										   rSize.width,
										   rSize.height);
				
				// Bound the path
				CGPathAddRect(pPath, NULL, bounds);
				
				// Layout the attributed string in a frame
				CTFrameRef pFrame = CTFramesetterCreateFrame(pFrameSetter, range, pPath, NULL);
				
				if( pFrame != NULL )
				{
					// Compute bounds for the bitmap context
					size_t width  = size_t(rSize.width);
					size_t height = size_t(rSize.height);
					size_t stride = sizeof(GLuint) * width;
					
					// No explicit backing-store allocation here.  We'll let the
					// context allocate the storage for us.
					pContext = CGBitmapContextCreate(NULL,
													 width,
													 height,
													 8,
													 stride,
													 pColorspace,
													 kCGImageAlphaPremultipliedLast);
					
					if( pContext != NULL )
					{
						// Use this for vertical reflection
						CGContextTranslateCTM(pContext, 0.0, height);
						CGContextScaleCTM(pContext, 1.0, -1.0);
						
						// Draw the frame into a bitmap context
						CTFrameDraw(pFrame, pContext);
						
						// Flush the context
						CGContextFlush(pContext);
					} // if
					
					// Release the frame
					CFRelease(pFrame);
				} // if
				
				CFRelease(pPath);
			} // if
			
			CFRelease(pFrameSetter);
		} // if
	} // if
    
    return pContext;
} // CGContextCreateFromAttributedString

// Create a bitmap context from a core foundation string, font,
// justification, and font size
static CGContextRef CGContextCreateFromString(CFStringRef pString,
                                              CTFontRef font,
                                              const CTTextAlignment nAlignment,
                                              CGColorRef fgColor,
                                              CGSize& rSize)
{
	CGContextRef pContext = NULL;
	
	if( pString != NULL )
	{
		// Get a generic linear RGB color space
		CGColorSpaceRef pColorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
		
		if( pColorspace != NULL )
		{
         // Creating a mutable attributed string
         CFRange range;
         
         CFMutableAttributedStringRef pAttrString = CFMutableAttributedStringCreate(pString,
                                                                                    font,
                                                                                    fgColor,
                                                                                    nAlignment,
                                                                                    &range);
         
         if( pAttrString != NULL )
         {
            // Create a context from our attributed string
            pContext = CGContextCreateFromAttributedString(pAttrString,
                                                           range,
                                                           pColorspace,
                                                           rSize);
            
            CFRelease(pAttrString);
         } // if
         
			CFRelease(pColorspace);
		} // if
	} // if
	
   return pContext;
} // CGContextCreateFromString

// Create a bitmap context from a core foundation string, font,
// justification, and font size
static CGContextRef CGContextCreateFromAttrString(CFMutableAttributedStringRef attrString, CFRange attrRange,
                                              CGSize& rSize)
{
	CGContextRef pContext = NULL;
	
   // Get a generic linear RGB color space
   CGColorSpaceRef pColorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
		
	if( pColorspace != NULL )
	{
      // Create a context from our attributed string
      pContext = CGContextCreateFromAttributedString(attrString, attrRange, pColorspace, rSize);
		CFRelease(pColorspace);
	} // if
	
   return pContext;
} // CGContextCreateFromString


#pragma mark -
#pragma mark Private - Utilities - OpenGL - Textures

// Create a 2D texture
static void GLTexture2DCreate(const GLuint texID,
                              const GLuint nWidth,
                              const GLuint nHeight,
                              const GLvoid * const pPixels)
{
    if( texID )
    {
		// Bind a texture with ID
        glBindTexture(GL_TEXTURE_2D, texID);
        // Initialize the texture
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA,
                     nWidth,
                     nHeight,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_INT_8_8_8_8_REV,
                     pPixels);
    } // if
} // GLTexture2DCreate

// Create a texture from a bitmap context
static void GLTexture2DCreateFromContext(GLuint texID, CGContextRef pContext)
{
    if( pContext != NULL )
    {
        GLuint nWidth  = GLuint(CGBitmapContextGetWidth(pContext));
        GLuint nHeight = GLuint(CGBitmapContextGetHeight(pContext));
        
        const GLvoid *pPixels = CGBitmapContextGetData(pContext);
        
        GLTexture2DCreate(texID, nWidth, nHeight, pPixels);
       GL_ERR_CHECK();
    } // if
    
} // GLTexture2DCreateFromContext

void GLTexture2DCreateFromStringFontRef(GLuint texID,
                                 CTFontRef font,
                                 CFStringRef text,
                                 const CTTextAlignment alignment,
                                 CGColorRef fgColor,
                                 CGSize& size)
{
   CGContextRef pCtx = CGContextCreateFromString(text,
                                                 font,
                                                 alignment,
                                                 fgColor,
                                                 size);
   
   if( pCtx != NULL )
   {
      GLTexture2DCreateFromContext(texID, pCtx);
      
      CGContextRelease(pCtx);
   } // if
   
} // GLTexture2DCreateFromString


void GLTexture2DCreateFromAttrString(GLuint texID,
                                     CFMutableAttributedStringRef attrString,
                                     CFRange attrRange,
                                     CGSize& size)
{
   CGContextRef pCtx = CGContextCreateFromAttrString(attrString,
                                                     attrRange,
                                                     size);
   
   if( pCtx != NULL )
   {
      GLTexture2DCreateFromContext(texID, pCtx);
      
      CGContextRelease(pCtx);
   } // if
   
} // GLTexture2DCreateFromString


void GLTexture2DAttrString(GLuint texID, CFMutableAttributedStringRef attrString, CFRange attrRange, glm::vec2& size)
{
   CGSize cgSize;
   
   GLTexture2DCreateFromAttrString(texID, attrString, attrRange, cgSize);
   size.x = cgSize.width;
   size.y = cgSize.height;
}

void GLTexture2DStringFontRef(GLuint texID,
                       CTFontRef font,
                       CFStringRef text,
                       const CTTextAlignment alignment,
                       CGColorRef fgColor,
                       glm::vec2& size)
{
   CGSize cgSize;
   
   GLTexture2DCreateFromStringFontRef(texID, font, text, alignment, fgColor, cgSize);
   size.x = cgSize.width;
   size.y = cgSize.height;
}
#endif