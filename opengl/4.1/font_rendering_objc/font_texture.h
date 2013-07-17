#ifndef _FONT_TEXTURE_H
#define _FONT_TEXTURE_H

#include <string>
#include <glm/glm.hpp>
#include "opengl.h"

class FontTextureOSX;

enum TextAlign
{
   TEXT_ALIGN_LEFT,
   TEXT_ALIGN_CENTER,
   TEXT_ALIGN_RIGHT,
   TEXT_ALIGN_JUSTIFIED
};

/**
 * Renders a string of text into an OpenGL texture map
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
   FontTexture(const std::string& font, const std::string& text, float pointSize, const glm::vec4& fgColor, const glm::vec4& bgColor, TextAlign align = TEXT_ALIGN_CENTER);

   /**
    * Destructor
    */
   ~FontTexture();
   
   /**
    * Sets the text to be rendered. This does not trigger an update of the texture map
    *
    * @param text
    *    The text to be rendered
    */
   void setText(const std::string& text);

   /**
    * Set the font to use for the text
    */
   void setFont(const std::string& font);
   
   /**
    * Update the texture map
    */
   void update();
   
   /**
    * @return the OpenGL texture handle
    */
   GLuint getID() const
   {
      return _id;
   }
   
   /**
    * @return size of texture map
    */
   glm::vec2 getSize() const;
   
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
    * Initialize implementation specific objects
    */
   void initImpl();
   
private:
   GLuint          _id;           //< Texture ID handle
   std::string     _font;         //< Name of the font
   std::string     _text;         //< Texture to render
   float           _pointSize;    //< Point size for the font
   glm::vec4       _fgColor;      //< Foreground color
   glm::vec4       _bgColor;      //< Background color
   glm::vec2       _texSize;      //< Size of texture in texels
   TextAlign       _align;        //< Text alignment method
   bool            _needsRefresh; //< true if the texture map need to be refreshed?
   FontTextureOSX* _impl;
};

#endif
