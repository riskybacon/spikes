#include "font_texture.h"

FontTexture::FontTexture(Font* font, const std::string& text)
: mTexID  (0)
, mFont   (font)
, mText   (text) 
{
}

void FontTexture::initGL(void)
{
  // Create the texture map / bitmap
  mFont->createBitmap(mText);
  
  // Get metrics - these change with every call to createBitmap,
  // so we'll need to remember them
  mBBoxWidth = mFont->boundingBoxWidth();
  mBBoxHeight = mFont->boundingBoxHeight();
  
  mTexWidth = mFont->bitmapWidth();
  mTexHeight = mFont->bitmapHeight();

  // Create the texture map
  glGenTextures(1, &mTexID);
  GL_ERR_CHECK();
  glBindTexture(GL_TEXTURE_2D, mTexID);
  GL_ERR_CHECK();
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  GL_ERR_CHECK();
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  GL_ERR_CHECK();
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  GL_ERR_CHECK();
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  GL_ERR_CHECK();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, mTexWidth, mTexHeight, 0, GL_RED, GL_UNSIGNED_BYTE, mFont->data());
  GL_ERR_CHECK();
  glGenerateMipmap(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
}
