/*
     File: OpenGLTexture.mm
 Abstract: 
 Utility toolkit for generating an OpenGL textures from strings.
 
  Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2013 Apple Inc. All Rights Reserved.
 
 */

#pragma mark -
#pragma mark Headers

#import <Cocoa/Cocoa.h>

// OpenGL utilities header
#import "OpenGLTexture.h"

#pragma mark -
#pragma mark Private - Utilities - CF

// Create an attributed string from a CF string, font, justification, and font size
static CFMutableAttributedStringRef CFMutableAttributedStringCreate(CFStringRef pString,
                                                                    CFStringRef pFontNameSrc,
                                                                    CGColorRef pComponents,
                                                                    const CGFloat nFontSize,
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
			// If the font name is null default to Helvetica
			CFStringRef pFontNameDst = (pFontNameSrc) ? pFontNameSrc : CFSTR("Helvetica");
			
			// Prepare font
			CTFontRef pFont = CTFontCreateWithName(pFontNameDst, nFontSize, NULL);
			
			if( pFont != NULL )
			{
				// Set attributed string properties
				const GLuint nCntDict = 3;
				
				CFStringRef keys[nCntDict] =
				{
					kCTParagraphStyleAttributeName,
					kCTFontAttributeName,
					kCTForegroundColorAttributeName
				};
				
				CFTypeRef values[nCntDict] =
				{
					pStyle,
					pFont,
					pComponents
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
				
				CFRelease(pFont);
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
                                                        NSSize &rSize)
{
	CGContextRef pContext = NULL;
	
	if( pAttrString != NULL )
	{
		// Acquire a frame setter
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
											  CFStringRef pFontName,
											  const CGFloat nFontSize,
											  const CTTextAlignment nAlignment,
											  const CGFloat * const pComponents,
											  NSSize &rSize)
{
	CGContextRef pContext = NULL;
	
	if( pString != NULL )
	{
		// Get a generic linear RGB color space
		CGColorSpaceRef pColorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
		
		if( pColorspace != NULL )
		{
			// Create a white color reference
			CGColorRef pColor = CGColorCreate(pColorspace, pComponents);
			
			if( pColor != NULL )
			{
				// Creating a mutable attributed string
				CFRange range;
				
				CFMutableAttributedStringRef pAttrString = CFMutableAttributedStringCreate(pString,
																						   pFontName,
																						   pColor,
																						   nFontSize,
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
				
				CFRelease(pColor);
			} // if
			
			CFRelease(pColorspace);
		} // if
	} // if
	
    return pContext;
} // CGContextCreateFromString

// Create a bitmap context from a c-string, font, justification, and font size
static CGContextRef CGContextCreateFromString(const GLchar * const pString,
											  const GLchar * const pFontName,
											  const CGFloat nFontSize,
											  const CTTextAlignment nAlignment,
											  const CGFloat * const pComponents,
											  NSSize &rSize)
{
	CGContextRef pContext = NULL;
	
	if( pString != NULL )
	{
		CFStringRef pCFString = CFStringCreateWithCString(kCFAllocatorDefault,
														  pString,
														  kCFStringEncodingASCII);
		
		if( pCFString != NULL )
		{
			const GLchar *pFontString = (pFontName) ? pFontName : "Helvetica";
			
			CFStringRef pFontCFString = CFStringCreateWithCString(kCFAllocatorDefault,
																  pFontString,
																  kCFStringEncodingASCII);
			
			if( pFontCFString != NULL )
			{
				pContext = CGContextCreateFromString(pCFString,
													 pFontCFString,
													 nFontSize,
													 nAlignment,
													 pComponents,
													 rSize);
				
				CFRelease(pFontCFString);
			} // if
			
			CFRelease(pCFString);
		} // if
	} // if
	
    return pContext;
} // CGContextCreateFromString

#pragma mark -
#pragma mark Private - Utilities - OpenGL - Textures

// Create a 2D texture
static GLuint GLTexture2DCreate(const GLuint nWidth,
                                const GLuint nHeight,
                                const GLvoid * const pPixels)
{
    GLuint nTID = 0;
    
    // Greate a texture
    glGenTextures(1, &nTID);
    
    if( nTID )
    {
		// Bind a texture with ID
        glBindTexture(GL_TEXTURE_2D, nTID);
        
        // Set texture properties (including linear mipmap)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
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
		
		// Generate mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
		
        // Discard
        glBindTexture(GL_TEXTURE_2D, 0);
    } // if
    
    return nTID;
} // GLTexture2DCreate

// Create a texture from a bitmap context
static GLuint GLTexture2DCreateFromContext(CGContextRef pContext)
{
    GLuint nTID = 0;
    
    if( pContext != NULL )
    {
        GLuint nWidth  = GLuint(CGBitmapContextGetWidth(pContext));
        GLuint nHeight = GLuint(CGBitmapContextGetHeight(pContext));
        
        const GLvoid *pPixels = CGBitmapContextGetData(pContext);
        
        nTID = GLTexture2DCreate(nWidth, nHeight, pPixels);
        
        // Was there a GL error?
        GLenum nErr = glGetError();
        
        if( nErr != GL_NO_ERROR )
        {
            NSLog(@">> OpenGL Error: %04x caught at %s:%u", nErr, __FILE__, __LINE__);
			
			glDeleteTextures(1, &nTID);
			
			nTID = 0;
        } // if
    } // if
    
    return nTID;
} // GLTexture2DCreateFromContext

#pragma mark -
#pragma mark Public - Constructors

// Generate a texture from a core foundation string, using a font, at a size,
// with alignment and color
static GLuint GLTexture2DCreateFromString(CFStringRef pString,
								   CFStringRef pFontName,
								   const CGFloat nFontSize,
								   const CTTextAlignment nAlignment,
								   const CGFloat * const pColor,
								   NSSize &rSize)
{
    GLuint nTID = 0;
    
    CGContextRef pCtx = CGContextCreateFromString(pString,
												  pFontName,
												  nFontSize,
												  nAlignment,
												  pColor,
												  rSize);
    
    if( pCtx != NULL )
    {
        nTID = GLTexture2DCreateFromContext(pCtx);
        
        CGContextRelease(pCtx);
    } // if
    
    return nTID;
} // GLTexture2DCreateFromString

GLuint GLTexture2DCreateFromString(const char* pString,
                                   const char* pFontName,
                                   const float nFontSize,
                                   TextAlignment nAlignment,
                                   const glm::vec4& color,
                                   glm::vec2& size
                                   )
{
   NSSize rSize;
   CTTextAlignment ctalign;
   
   CFStringRef pCFString = CFStringCreateWithCString(kCFAllocatorDefault,
                                                     pString,
                                                     kCFStringEncodingASCII);

   const GLchar *pFontString = (pFontName) ? pFontName : "Helvetica";
   
   CFStringRef pFontCFString = CFStringCreateWithCString(kCFAllocatorDefault,
                                                         pFontString,
                                                         kCFStringEncodingASCII);

   CGFloat pColor[4];
   pColor[0] = color.r;
   pColor[1] = color.g;
   pColor[2] = color.b;
   pColor[3] = color.a;
   
   switch (nAlignment)
   {
      case CENTER:
         ctalign = kCTTextAlignmentCenter;
         break;

      case LEFT:
         ctalign = kCTTextAlignmentLeft;
         break;

      case RIGHT:
         ctalign = kCTTextAlignmentRight;
         break;

      default:
         ctalign = kCTTextAlignmentLeft;
         break;
   }
   
   GLuint id = GLTexture2DCreateFromString(pCFString, pFontCFString, nFontSize, ctalign, pColor, rSize);
   
   size.x = rSize.width;
   size.y = rSize.height;
   return id;
}

#if 0
// Generate a texture from a stl string, using a font, at a size,
// with an alignment and a color
GLuint GLTexture2DCreateFromString(const GLstring &rString,
								   const GLstring &rFontName,
								   const CGFloat nFontSize,
								   const CTTextAlignment nAlignment,
								   const CGFloat * const pColor,
								   NSSize &rSize)
{
    return GLTexture2DCreateFromString(rString.c_str(),
                                       rFontName.c_str(),
                                       nFontSize,
                                       nAlignment,
                                       pColor,
                                       rSize);
} // GLTexture2DCreateFromString
#endif