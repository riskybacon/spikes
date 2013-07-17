#include "font_texture.h"
#include "font_texture_osx.h"

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
FontTexture::FontTexture(const std::string& font, const std::string& text, float pointSize,
                         const glm::vec4& fgColor, const glm::vec4& bgColor,
                         TextAlign align)
: _font         (font)
, _text         (text)
, _pointSize    (pointSize)
, _fgColor      (fgColor)
, _bgColor      (bgColor)
, _align        (align)
, _needsRefresh (true)
{
   initGL();
#if 0
   _impl = new FontTextureOSX(_id, font, text, pointSize, fgColor, align);
#endif
   //   update();
}

/**
 * Destructor
 */
FontTexture::~FontTexture()
{
   freeGL();
#if 0
   delete _impl;
#endif
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

/**
 * Updates the texture map
 */
void FontTexture::update()
{
#if 1
   GLTexture2DString(_id, _text, _font, _pointSize, _align, _fgColor, _texSize);
   // The texture is bound upon return from the function
   glGenerateMipmap(GL_TEXTURE_2D);
   _needsRefresh = false;
#else
   _impl->update();
#endif
}

/**
 * Sets the text to be rendered. This does not trigger an update of the texture map
 *
 * @param text
 *    The text to be rendered
 */
void FontTexture::setText(const std::string& text)
{
#if 1
   _text = text;
   _needsRefresh = true;
#else
   _impl->setText(text);
#endif
}

/**
 * Set the font to use for the text
 */
void FontTexture::setFont(const std::string& font)
{
#if 1
   _font = font;
   _needsRefresh = true;
#else
   _impl->setFont(font, _pointSize);
#endif
}

/**
 * @return size of texture map
 */
glm::vec2 FontTexture::getSize() const
{
#if 1
   return _texSize;
#else
   return _impl->getSize();
#endif

}
