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
   _impl = new FontTextureOSX(font, text, pointSize, fgColor, align);
}

/**
 * Destructor
 */
FontTexture::~FontTexture()
{
   delete _impl;
}

GLuint FontTexture::getID() const
{
   return _impl->getID();
}

/**
 * Updates the texture map
 */
void FontTexture::update()
{
   _impl->update();
}

/**
 * Sets the text to be rendered. This does not trigger an update of the texture map
 *
 * @param text
 *    The text to be rendered
 */
void FontTexture::setText(const std::string& text)
{
#if 0
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
#if 0
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
#if 0
   return _texSize;
#else
   return _impl->getSize();
#endif

}
