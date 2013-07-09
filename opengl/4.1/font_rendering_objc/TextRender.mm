// CPP interface to Objective-C
#include "TextRender.h"
#include <iostream>
#include <glm/glm.hpp>

#import "TextRenderObjC.h"
#import <Cocoa/Cocoa.h>

struct TextRenderImpl
{
   TextRenderObjC* wrapped;
};

TextRender::TextRender()
: impl(new TextRenderImpl)
{
   impl->wrapped = [[TextRenderObjC alloc] init];
}

TextRender::~TextRender()
{
   if(impl)
   {
      [impl->wrapped release];
   }
   delete impl;
}

#if 0
// Create an attributed string from a CF string, font, justification, and font size
CFMutableAttributedStringRef CFMutableAttributedStringCreate(CFStringRef pString,
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
} // CFMutableAttributedStringCre

// Create a bitmap context from a string, font, justification, and font size
static CGContextRef CGContextCreateFromAttributedString(const CFRange &rRange,
                                                        CGColorSpaceRef pColorspace,
                                                        NSSize &rSize)
{
   
   CFStringRef pString;
   CFStringRef pFontName;
   CGFloat nFontSize = 20.0f;
   CTTextAlignment nAlignment = kCTTextAlignmentCenter;
   CFRange range;
   CGColorRef pComponents;
   
   CFMutableAttributedStringRef pAttrString =
   
   CFMutableAttributedStringCreate(pString,
                                   pFontName,
                                   pComponents,
                                   nFontSize,
                                   nAlignment,
                                   &range);
   
   
   // Get size of bitmap
   
   CTFramesetterCreateFrame(<#CTFramesetterRef framesetter#>, <#CFRange stringRange#>, <#CGPathRef path#>, <#CFDictionaryRef frameAttributes#>)
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
} // CGContextCreateFromString



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
} // CGContextCreateFromString



void TextRender::renderString(const char* text, const char* fontName, float fontSize)
{
   CGColorSpaceRef colorspace; // todo
   CGContextRef context;
   NSSize size;

   CGColorRef components; // todo
   
   CTTextAlignment alignment = kCTTextAlignmentCenter;
   CFRange range;

   CFStringRef cfstringText = CFStringCreateWithCString(kCFAllocatorDefault,
                                                        text,
                                                        kCFStringEncodingASCII);
   CFStringRef cfstringFontName = CFStringCreateWithCString(kCFAllocatorDefault, fontName, kCFStringEncodingASCII);

   // Create attributed string
   CFMutableAttributedStringRef attrString = CFMutableAttributedStringCreate(cfstringText,
                                                                             cfstringFontName,
                                                                             components,
                                                                             fontSize,
                                                                             alignment,
                                                                             &range);

   
   
   // Acquire a frame setter
   CTFramesetterRef pFrameSetter = CTFramesetterCreateWithAttributedString(attrString);

   // Create a path for layout
   CGMutablePathRef pPath = CGPathCreateMutable();

   CGSize  constraint = CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX);
   
   // Get the CoreText suggested size from our framesetter
   size = CTFramesetterSuggestFrameSizeWithConstraints(pFrameSetter,
                                                       range,
                                                       NULL,
                                                       constraint,
                                                       &range);
   
   // Set path bounds
   CGRect bounds = CGRectMake(0.0f,
                              0.0f,
                              size.width,
                              size.height);
   
   // Bound the path
   CGPathAddRect(pPath, NULL, bounds);
   
   // Layout the attributed string in a frame
   CTFrameRef pFrame = CTFramesetterCreateFrame(pFrameSetter, range, pPath, NULL);

   // Compute bounds for the bitmap context
   size_t width  = size_t(size.width);
   size_t height = size_t(size.height);
   size_t stride = sizeof(GLuint) * width;
   
   // No explicit backing-store allocation here.  We'll let the
   // context allocate the storage for us.
   context = CGBitmapContextCreate(NULL,
                                   width,
                                   height,
                                   8,
                                   stride,
                                   pColorspace,
                                   kCGImageAlphaPremultipliedLast);

   // Render text into context
   
   // Use this for vertical reflection
   CGContextTranslateCTM(pContext, 0.0, height);
   CGContextScaleCTM(pContext, 1.0, -1.0);
   
   // Draw the frame into a bitmap context
   CTFrameDraw(pFrame, pContext);
   
   // Flush the context
   CGContextFlush(pContext);

   // Release the frame
   CFRelease(pFrame);
   CFRelease(pPath);
   CFRelease(pFrameSetter);
   // Get pointer to bitmap data
   
   // Load into texture
   
}
#endif

void TextRender::reticulate()
{
   const char inText[] = "Cocoa";
   const char inFontName[] = "Menlo";
   
   NSString* text     = [NSString stringWithCString:inText encoding:NSUTF8StringEncoding];
   NSString* fontName = [NSString stringWithCString:inFontName encoding:NSUTF8StringEncoding];
   float     fontSize = 20.0f;
   NSImage*  textImage;
   
   NSSize textSize;
   NSSize textureSize;
   
   NSPoint delta;
   
   glm::vec4 fg(0, 0, 0, 1);
   glm::vec4 bg(1, 1, 1, 1);
   
   NSMutableDictionary* textAttrib = [[NSMutableDictionary alloc] init];

   NSColor				*fColor = [NSColor colorWithCalibratedRed: fg.r
                                                  green: fg.g
                                                   blue: fg.b
                                                  alpha: fg.a];
   NSColor				*bColor = [NSColor colorWithCalibratedRed: bg.r
                                                  green: bg.g
                                                   blue: bg.b
                                                  alpha: bg.a];

   [textAttrib setObject: [NSFont fontWithName: fontName size: fontSize] forKey: NSFontAttributeName];
   [textAttrib setObject: fColor forKey: NSForegroundColorAttributeName];
   [textAttrib setObject: bColor forKey: NSBackgroundColorAttributeName];

   textSize = [text sizeWithAttributes: textAttrib];
   textureSize = textSize;
   
   // Calculate the delta to center the text in the texture
   delta.x		= (textureSize.width - textSize.width) / 2.0;
   delta.y		= (textureSize.height - textSize.height) / 2.0;

   textImage = [[[NSImage alloc] initWithSize: textSize] autorelease];
   [textImage lockFocus];
   [textImage setBackgroundColor:bColor];
   NSEraseRect(NSMakeRect(0.0f, 0.0f, textSize.width, textSize.height));
   
   [text drawInRect: NSMakeRect(delta.x, delta.y, textSize.width, textSize.height) withAttributes: textAttrib];

#if 0
   const GLvoid *pPixels = CGBitmapContextGetData(pContext);
#endif
   [textImage unlockFocus];
   [textAttrib release];
   
   std::cout << textSize.width << "," << textSize.height << std::endl;
   
   NSUInteger                              width, height, i;
   CGContextRef                    context;
   void*                                   data;
   CGColorSpaceRef                 colorSpace;
   NSFont *                                font;
   
   //   font = [UIFont fontWithName:name size:size];
   
   
   
   std::cout << "TextRender::reticulate" << std::endl;
   [impl->wrapped reticulate];
}

