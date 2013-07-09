#ifndef _text_render_h
#define _text_render_h

struct TextRenderImpl;

class TextRender
{
public:
   TextRender();
   ~TextRender();
   void reticulate();
   //   void renderString(const char* text, const char* fontName, float fontSize);

private:
   TextRenderImpl* impl;
};

#endif

