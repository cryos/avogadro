/**********************************************************************
  TextRenderer - a temporary replacement for QGLWidget::renderText until it
  matures a bit more ;)

  Copyright (C) 2007 Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "textrenderer.h"

#include<QPainter>
#include<avogadro/glwidget.h>
#include<avogadro/camera.h>

const int TEXT_OUTLINE_INTENSITY = 128;

namespace Avogadro {

/** @internal
* This is a helper class for TextRenderer.
*
* The CharRenderer class represents a character stored as OpenGL rendering
* data : a texture object and a display list mapping it on a quad and then
* translating to the right of it.
*
* See the charTable member of TextRenderer for an example of use of
* this class.
*/
class CharRenderer
{
  protected:
    /**
     * The OpenGL texture object
     */
    GLuint m_texture;

    /**
     * The OpenGL display list
     */
    GLuint m_displayList;

    /**
     * Width and height in pixels of the rendered character
     */
    int m_width, m_height;

  public:
    CharRenderer();
    ~CharRenderer();
    
    /** Builds the texture and the display list for a given character and font */
    bool initialize( QChar c, const QFont &font );
    
    /** Calls the display list, drawing the character as a textured quad */
    inline void draw() const
    {
      glCallList( m_displayList );
    }
    
    /** @returns the height of the rendered character in pixels */
    inline int height() const { return m_height; }
    
    /** @returns the width of the rendered character in pixels */
    inline int width() const { return m_width; }
};

CharRenderer::CharRenderer()
{
  m_texture = 0;
  m_displayList = 0;
}

CharRenderer::~CharRenderer()
{
  if( m_texture ) glDeleteTextures( 1, &m_texture );
  if( m_displayList ) glDeleteLists( m_displayList, 1 );
}

bool CharRenderer::initialize( QChar c, const QFont &font )
{
  if( m_displayList ) return true;
  
  // *** STEP 1 : render the character to a QImage ***
  
  // compute the size of the image to create
  QFontMetrics fontMetrics ( font );
  m_width = fontMetrics.width( c ) + 2;
  m_height = fontMetrics.height() + 2;
  if( m_width <= 2 || m_height <= 2 ) return false;
  // create a new image
  QImage image( m_width, m_height, QImage::Format_RGB32 );
  QPainter painter;
  // start painting the image
  painter.begin( &image );
  painter.setFont( font );
  painter.setRenderHint( QPainter::TextAntialiasing );
  painter.setBackground( Qt::black );
  painter.eraseRect( image.rect() );
  // use an artificial blue color. This image is only used internally anyway.
  painter.setPen( Qt::blue );
  // actually paint the character. The position seems right at least with Helvetica
  // at various sizes, I didn't try other fonts. If in the future a user complains about
  // the text being clamped to the top/bottom, change this line.
  painter.drawText ( 1, m_height - painter.fontMetrics().descent(), c );
  // end painting the image
  painter.end();

  // *** STEP 2 : extract the raw bitmap from the image ***
  
  // --> explanation: the image we just rendered is RGB, but actually all the
  //     data is in the B channel because we painted in blue. Now we extract
  //     this blue channel into a separate bitmap that'll be faster to manipulate
  //     in what follows.

  int *rawbitmap = new int[ m_width * m_height ];
  if( ! rawbitmap ) return false;
  int n = 0;
  // loop over the pixels of the image, in reverse y direction
  for( int j = m_height - 1; j >= 0; j-- )
  for( int i = 0; i < m_width; i++, n++ )
  {
    rawbitmap[n] = qBlue( image.pixel( i, j ) );
  }
  
  // *** STEP 3 : compute the neighborhood map from the raw bitmap ***
  
  // --> explanation: we apply a convolution filter to the raw bitmap
  //     to produce a new map each pixel is associated a float telling how
  //     much it is surrounded by other pixels.

  int *neighborhood = new int[ m_width * m_height ];
  if( ! neighborhood ) return false;

  // the weight of diagonal-adjactent pixels. Side-adjacent pixels have
  // weight 256 (their value is shifted 8 bits on the left, hence multiplied
  // by 236). The value below is approximately 256/sqrt(2), as suggested by
  // Pythagora's theorem.
  const int df = 181;

  // first compute the pixels that are not on an edge of the bitmap
  for( int j = 1; j < m_height - 1; j++ )
  for( int i = 1; i < m_width - 1; i++ )
  {
    n = i + j * m_width;
    neighborhood[n]
            = ( ( rawbitmap[n - m_width - 1]
                + rawbitmap[n - m_width + 1]
                + rawbitmap[n + m_width - 1]
                + rawbitmap[n + m_width + 1] ) * df )
            + ( ( rawbitmap[n - m_width]
                + rawbitmap[n - 1]
                + rawbitmap[n + 1]
                + rawbitmap[n + m_width] ) << 8 );
  }
  
  // compute the pixels on the top and bottom edges, minus the 4 corners
  for( int i = 1; i < m_width - 1; i++ )
  {
    n = i;
    neighborhood[n] = ( (rawbitmap[n - 1] + rawbitmap[n + 1] + rawbitmap[n + m_width]) << 8 )
                    + ( (rawbitmap[n + m_width - 1] + rawbitmap[n + m_width + 1]) * df );

    n = i + (m_height - 1) * m_width;
    neighborhood[n] = ( (rawbitmap[n - m_width] + rawbitmap[n - 1] + rawbitmap[n + 1]) << 8 )
                    + ( (rawbitmap[n - m_width - 1] + rawbitmap[n - m_width + 1]) * df );
  }

  // compute the pixels on the left and right edges, minus the 4 corners
  for( int j = 1; j < m_height - 1; j++ )
  {
    n = j * m_width;
    neighborhood[n] = ( (rawbitmap[n - m_width] + rawbitmap[n + 1] + rawbitmap[n + m_width]) << 8 )
                    + ( (rawbitmap[n - m_width + 1] + rawbitmap[n + m_width + 1]) * df );

    n = m_width - 1 + j * m_width;
    neighborhood[n] = ( (rawbitmap[n - m_width] + rawbitmap[n - 1] + rawbitmap[n + m_width]) << 8 )
                    + ( (rawbitmap[n - m_width - 1] + rawbitmap[n + m_width - 1]) * df );
  }

  // compute the 4 corners
  neighborhood[0] = ( ( rawbitmap[1] + rawbitmap[m_width] ) << 8 )
                  + rawbitmap[m_width] * df;
  neighborhood[m_width-1] = ( ( rawbitmap[m_width-2] + rawbitmap[2*m_width-1] ) << 8 )
                          + rawbitmap[2*m_width-2] * df;
  neighborhood[(m_height-1)*m_width]
      = ( ( rawbitmap[(m_height-2)*m_width] + rawbitmap[(m_height-1)*m_width+1] ) << 8 )
      + rawbitmap[(m_height-2)*m_width+1] * df;
  neighborhood[(m_height-1)*m_width+m_width-1]
      = ( ( rawbitmap[(m_height-1)*m_width+m_width-2]
          + rawbitmap[(m_height-2)*m_width+m_width-1] ) << 8 )
      + rawbitmap[(m_height-2)*m_width+m_width-2] * df;

  // *** STEP 4 : compute the final bitmap ***
  // --> explanation: we build the bitmap that will be passed to OpenGL for texturing.
  //     this texture has 2 channels: the luminance and the alpha channels.
  //     the rawbitmap readily gives the luminance channel, while the computation of the
  //     alpha channel is a bit more involved and uses the neighborhood map.

  GLubyte *finalbitmap = new GLubyte[ 2 * m_width * m_height ];
  if( ! finalbitmap ) return false;

  for( int j = 0; j < m_height; j++ )
  for( int i = 0; i < m_width; i++ )
  {
    n = i + j * m_width;
    finalbitmap[2 * n] = rawbitmap[n];
    int alpha = ((TEXT_OUTLINE_INTENSITY * neighborhood[n]) >> 16) + rawbitmap[n];
    if( alpha > 255 ) {
      alpha = 255;
    }
    finalbitmap[2 * n + 1] = static_cast<GLubyte>(alpha);
  }
  
  // *** STEP 5 : pass the final bitmap to OpenGL for texturing ***

  glGenTextures( 1, &m_texture );
  if( ! m_texture ) return false;

  glBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_texture );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glTexImage2D(
    GL_TEXTURE_RECTANGLE_ARB,
    0,
    GL_LUMINANCE_ALPHA,
    m_width,
    m_height,
    0,
    GL_LUMINANCE_ALPHA,
    GL_UNSIGNED_BYTE,
    finalbitmap );
  glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  // the texture data is now kept alive by OpenGL. It's time to free the bitmaps.
  delete [] rawbitmap;
  delete [] neighborhood;
  delete [] finalbitmap;
  
  // *** STEP 6 : compile the display list ***

  m_displayList = glGenLists(1);
  if( ! m_displayList ) return false;

  glNewList( m_displayList, GL_COMPILE );
  glBindTexture( GL_TEXTURE_RECTANGLE_ARB, m_texture );
  // paint the character
  glBegin( GL_QUADS );
  glTexCoord2i( 0, 0);
  glVertex2f( 0 , -m_height );
  glTexCoord2i( m_width, 0);
  glVertex2f( m_width , -m_height );
  glTexCoord2i( m_width, m_height);
  glVertex2f( m_width, 0 );
  glTexCoord2i( 0, m_height);
  glVertex2f( 0 , 0 );
  glEnd();
  // move to the right so that the next character (if any) is paintd to the right.
  // this makes me think that we might consider allowing right-to-left text.
  // this would be needed for i18n, for example for the Arab language.
  glTranslatef( m_width-2, 0, 0 );
  glEndList();
  return true;
}

class TextRendererPrivate
{
  public:
  
    TextRendererPrivate() {}
    ~TextRendererPrivate() {}
  
    /**
    * The font used for rendering the chars.
    */
    QFont font;
    
    /**
    * This hash gives the correspondence table between QChars
    * (the keys) and the corresponding CharRenderers (the values).
    * Every time a QChar is being met, either it is found in this
    * table, in which case it can be directly rendered, or it is
    * not found, in which case a new CharRenderer is created for
    * it and added to this table.
    */
    QHash<QChar, CharRenderer*> charTable;

    /**
    * The GLWidget in which to render. This is set
    * once and for all by setup().
    */
    GLWidget *glwidget;

    GLboolean textmode;

    ///{ Members used to remember the OpenGL state in order to be able to restore it after rendering. See do_end().
    GLboolean wasEnabled_LIGHTING;
    GLboolean wasEnabled_TEXTURE_RECTANGLE_ARB;
    GLboolean wasEnabled_FOG;
    GLboolean wasEnabled_BLEND;
    ///}
};

TextRenderer::TextRenderer() : d(new TextRendererPrivate)
{
  d->glwidget = 0;
  d->textmode = false;
}

TextRenderer::~TextRenderer()
{
  QHash<QChar, CharRenderer *>::iterator i = d->charTable.begin();
  while( i != d->charTable.end() )
  {
    delete i.value();
    i = d->charTable.erase(i);
  }
  delete d;
}

void TextRenderer::setGLWidget( GLWidget *glwidget )
{
  d->glwidget = glwidget;
  d->font = d->glwidget->font();
}

void TextRenderer::begin()
{
  assert(d->glwidget);
  d->textmode = true;
  d->wasEnabled_LIGHTING = glIsEnabled( GL_LIGHTING );
  d->wasEnabled_FOG = glIsEnabled( GL_FOG );
  d->wasEnabled_TEXTURE_RECTANGLE_ARB = glIsEnabled( GL_TEXTURE_RECTANGLE_ARB );
  d->wasEnabled_BLEND = glIsEnabled( GL_BLEND );
  glDisable( GL_LIGHTING );
  glDisable( GL_FOG );
  glEnable( GL_TEXTURE_RECTANGLE_ARB );
  glEnable( GL_BLEND );
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0, d->glwidget->width(), 0, d->glwidget->height(), 0, 1 );
  glMatrixMode( GL_MODELVIEW );
}

void TextRenderer::end()
{
  assert(d->textmode);
  if( ! d->wasEnabled_TEXTURE_RECTANGLE_ARB )
  glDisable( GL_TEXTURE_RECTANGLE_ARB );
  if( ! d->wasEnabled_BLEND ) glDisable( GL_BLEND );
  if( d->wasEnabled_LIGHTING ) glEnable( GL_LIGHTING );
  if( d->wasEnabled_FOG ) glEnable( GL_FOG );
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  d->textmode = false;
}

int TextRenderer::do_draw( const QString &string )
{
  CharRenderer *c;
  int retval = 0;
  if( d->charTable.contains( string[0] ) )
  {
    c = d->charTable.value( string[0] );
    c->draw();
  }
  else
  {
    c = new CharRenderer;
    if( c->initialize( string[0], d->font ) )
    {
      d->charTable.insert( string[0], c);
      c->draw();
    }
    else
    {
      delete c;
      c = 0;
    }
  }
  if(c) retval = c->height() -2;

  for( int i = 1; i < string.size(); i++ )
  {
    if( d->charTable.contains( string[i] ) )
      d->charTable.value( string[i] )->draw();
    else
    {
      CharRenderer *c = new CharRenderer;
      if( c->initialize( string[i], d->font ) )
      {
        d->charTable.insert( string[i], c);
        c->draw();
      }
      else delete c;
    }
  }
  return retval;
}

int TextRenderer::draw( int x, int y, const QString &string )
{
  assert(d->textmode);
  if( string.isEmpty() ) return 0;

  glPushMatrix();
  glLoadIdentity();
  glTranslatef( x, d->glwidget->height() - y, 0 );
        int retval = do_draw(string);
  glPopMatrix();

  return retval;
}

int TextRenderer::draw( const Eigen::Vector3d &pos, const QString &string )
{
  assert(d->textmode);
  if( string.isEmpty() ) return 0;

  Eigen::Vector3d wincoords = d->glwidget->camera()->project(pos);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef( static_cast<int>(wincoords.x()),
                static_cast<int>(wincoords.y()),
                -wincoords.z() );
  int retval = do_draw(string);
  glPopMatrix();

  return retval;
}

} // namespace Avogadro
