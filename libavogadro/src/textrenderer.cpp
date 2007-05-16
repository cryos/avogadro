/**********************************************************************
  TextRenderer - a temporary replacement for QGLWidget::renderText until it
  matures a bit more ;)

  Copyright (C) 2007 Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation; either version 2 of the License, or 
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

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
    int m_realwidth, m_realheight;

  public:
    CharRenderer();
    ~CharRenderer();
    
    /** Builds the texture and the display list for a given character and font */
    bool initialize( QChar c, const QFont &font, GLenum textureTarget );
    
    /** Calls the display list, drawing the character as a textured quad */
    inline void draw() const
    {
      glCallList( m_displayList );
    }
    
    /** @returns the height of the rendered character in pixels */
    inline int height() const { return m_realheight; }
    
    /** @returns the width of the rendered character in pixels */
    inline int width() const { return m_realwidth; }
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

bool CharRenderer::initialize( QChar c, const QFont &font, GLenum textureTarget )
{
  if( m_displayList ) return true;
  
  // *** STEP 1 : render the character to a QImage ***
  
  // compute the size of the image to create
  QFontMetrics fontMetrics ( font );
  m_realwidth = fontMetrics.width(c);
  m_realheight = fontMetrics.height();
  if(m_realwidth == 0 || m_realheight == 0) return false;
  int texwidth  =  m_realwidth + 2;
  int texheight = m_realheight + 2;
  if( textureTarget == GL_TEXTURE_2D )
  {
    int x = qMax( texwidth, texheight );
    // find next power of two
    int n;
    for(n = 1; n < x; n = n << 1);
    // the texture must be square, and its size must be a power of two.
    texwidth = texheight = n;
  }
  // create a new image
  QImage image( texwidth, texheight, QImage::Format_RGB32 );
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
  painter.drawText ( 1, m_realheight + 2 - painter.fontMetrics().descent(), c );
  // end painting the image
  painter.end();

  // *** STEP 2 : extract the raw bitmap from the image ***
  
  // --> explanation: the image we just rendered is RGB, but actually all the
  //     data is in the B channel because we painted in blue. Now we extract
  //     this blue channel into a separate bitmap that'll be faster to manipulate
  //     in what follows.

  int *rawbitmap = new int[ texwidth * texheight ];
  if( ! rawbitmap ) return false;
  int n = 0;
  // loop over the pixels of the image, in reverse y direction
  for( int j = texheight - 1; j >= 0; j-- )
  for( int i = 0; i < texwidth; i++, n++ )
  {
    rawbitmap[n] = qBlue( image.pixel( i, j ) );
  }
  
  // *** STEP 3 : compute the neighborhood map from the raw bitmap ***
  
  // --> explanation: we apply a convolution filter to the raw bitmap
  //     to produce a new map each pixel is associated a float telling how
  //     much it is surrounded by other pixels.

  int *neighborhood = new int[ texwidth * texheight ];
  if( ! neighborhood ) return false;

  // the weight of diagonal-adjactent pixels. Side-adjacent pixels have
  // weight 256 (their value is shifted 8 bits on the left, hence multiplied
  // by 236). The value below is approximately 256/sqrt(2), as suggested by
  // Pythagora's theorem.
  const int df = 181;

  // first compute the pixels that are not on an edge of the bitmap
  for( int j = 1; j < texheight - 1; j++ )
  for( int i = 1; i < texwidth - 1; i++ )
  {
    n = i + j * texwidth;
    neighborhood[n]
            = ( ( rawbitmap[n - texwidth - 1]
                + rawbitmap[n - texwidth + 1]
                + rawbitmap[n + texwidth - 1]
                + rawbitmap[n + texwidth + 1] ) * df )
            + ( ( rawbitmap[n - texwidth]
                + rawbitmap[n - 1]
                + rawbitmap[n + 1]
                + rawbitmap[n + texwidth] ) << 8 );
  }
  
  // compute the pixels on the top and bottom edges, minus the 4 corners
  for( int i = 1; i < texwidth - 1; i++ )
  {
    n = i;
    neighborhood[n] = ( (rawbitmap[n - 1] + rawbitmap[n + 1] + rawbitmap[n + texwidth]) << 8 )
                    + ( (rawbitmap[n + texwidth - 1] + rawbitmap[n + texwidth + 1]) * df );

    n = i + (texheight - 1) * texwidth;
    neighborhood[n] = ( (rawbitmap[n - texwidth] + rawbitmap[n - 1] + rawbitmap[n + 1]) << 8 )
                    + ( (rawbitmap[n - texwidth - 1] + rawbitmap[n - texwidth + 1]) * df );
  }

  // compute the pixels on the left and right edges, minus the 4 corners
  for( int j = 1; j < texheight - 1; j++ )
  {
    n = j * texwidth;
    neighborhood[n] = ( (rawbitmap[n - texwidth] + rawbitmap[n + 1] + rawbitmap[n + texwidth]) << 8 )
                    + ( (rawbitmap[n - texwidth + 1] + rawbitmap[n + texwidth + 1]) * df );

    n = texwidth - 1 + j * texwidth;
    neighborhood[n] = ( (rawbitmap[n - texwidth] + rawbitmap[n - 1] + rawbitmap[n + texwidth]) << 8 )
                    + ( (rawbitmap[n - texwidth - 1] + rawbitmap[n + texwidth - 1]) * df );
  }

  // compute the 4 corners
  neighborhood[0] = ( ( rawbitmap[1] + rawbitmap[texwidth] ) << 8 )
                  + rawbitmap[texwidth] * df;
  neighborhood[texwidth-1] = ( ( rawbitmap[texwidth-2] + rawbitmap[2*texwidth-1] ) << 8 )
                          + rawbitmap[2*texwidth-2] * df;
  neighborhood[(texheight-1)*texwidth]
      = ( ( rawbitmap[(texheight-2)*texwidth] + rawbitmap[(texheight-1)*texwidth+1] ) << 8 )
      + rawbitmap[(texheight-2)*texwidth+1] * df;
  neighborhood[(texheight-1)*texwidth+texwidth-1]
      = ( ( rawbitmap[(texheight-1)*texwidth+texwidth-2]
          + rawbitmap[(texheight-2)*texwidth+texwidth-1] ) << 8 )
      + rawbitmap[(texheight-2)*texwidth+texwidth-2] * df;

  // *** STEP 4 : compute the final bitmap ***
  // --> explanation: we build the bitmap that will be passed to OpenGL for texturing.
  //     this texture has 2 channels: the luminance and the alpha channels.
  //     the rawbitmap readily gives the luminance channel, while the computation of the
  //     alpha channel is a bit more involved and uses the neighborhood map.

  GLubyte *finalbitmap = new GLubyte[ 2 * texwidth * texheight ];
  if( ! finalbitmap ) return false;

  for( n = 0; n < texheight * texwidth; n++ )
  {
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

  glBindTexture( textureTarget, m_texture );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glTexImage2D(
    textureTarget,
    0,
    GL_LUMINANCE_ALPHA,
    texwidth,
    texheight,
    0,
    GL_LUMINANCE_ALPHA,
    GL_UNSIGNED_BYTE,
    finalbitmap );
  glTexParameteri( textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  // the texture data is now kept alive by OpenGL. It's time to free the bitmaps.
  delete [] rawbitmap;
  delete [] neighborhood;
  delete [] finalbitmap;
  
  // *** STEP 6 : compile the display list ***

  m_displayList = glGenLists(1);
  if( ! m_displayList ) return false;

  glNewList( m_displayList, GL_COMPILE );
  glBindTexture( textureTarget, m_texture );
  int texcoord_width = (textureTarget == GL_TEXTURE_2D) ? 1 : texwidth;
  int texcoord_height = (textureTarget == GL_TEXTURE_2D) ? 1 : texheight;
  
  // paint the character
  glBegin( GL_QUADS );
  glTexCoord2i( 0, 0);
  glVertex2f( 0 , -texheight );
  glTexCoord2i( texcoord_width, 0);
  glVertex2f( texwidth , -texheight );
  glTexCoord2i( texcoord_width, texcoord_height);
  glVertex2f( texwidth, 0 );
  glTexCoord2i( 0, texcoord_height);
  glVertex2f( 0 , 0 );
  glEnd();
  // move to the right so that the next character (if any) is paintd to the right.
  // this makes me think that we might consider allowing right-to-left text.
  // this would be needed for i18n, for example for the Arab language.
  glTranslatef( m_realwidth, 0, 0 );
  glEndList();
  return true;
}

class TextRendererPrivate
{
  public:
  
    TextRendererPrivate() : initialized(false) {}
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
    
    bool initialized;
    
    GLenum textureTarget;
    static int isGLExtensionSupported(const char *extension);
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

// the code of isGLExtensionSupported() is taken from the webpage:
// http://www.opengl.org/resources/features/OGLextensions/
// it seems pretty clear from this context that this code is provided
// without any use restriction.
int TextRendererPrivate::isGLExtensionSupported(const char *extension)
{
  const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;
  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;
  extensions = glGetString(GL_EXTENSIONS);
  /* It takes a bit of care to be fool-proof about parsing the
      OpenGL extensions string. Don't be fooled by sub-strings,
      etc. */
  start = extensions;
  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return 1;
    start = terminator;
  }
  return 0;
}


void TextRenderer::begin()
{
  if(!d->initialized) {
    if(TextRendererPrivate::isGLExtensionSupported("GL_ARB_texture_rectangle")) {
      d->textureTarget = GL_TEXTURE_RECTANGLE_ARB;
      qDebug() << "OpenGL extension GL_ARB_texture_rectangle is present.";
    } else {
      d->textureTarget = GL_TEXTURE_2D;
      qDebug() << "OpenGL extension GL_ARB_texture_rectangle is absent.";
    }
    d->initialized = true;
  }

  assert(d->glwidget);
  d->textmode = true;
  glPushAttrib( GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT );
  glDisable(GL_LIGHTING);
  glDisable(GL_FOG);
  glEnable(d->textureTarget);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0, d->glwidget->width(), 0, d->glwidget->height(), 0, 1 );
  glMatrixMode( GL_MODELVIEW );
}

void TextRenderer::end()
{
  assert(d->textmode);
  glPopAttrib();
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
    if( c->initialize( string[0], d->font, d->textureTarget ) )
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
      if( c->initialize( string[i], d->font, d->textureTarget ) )
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
