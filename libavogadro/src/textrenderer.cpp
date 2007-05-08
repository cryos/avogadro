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

#include<avogadro/textrenderer.h>
#include<QPainter>
#include<avogadro/glwidget.h>
#include<avogadro/camera.h>

namespace Avogadro {

/**
* This is a helper class for TextRenderer.
*
* The CharRenderer class represents a character stored as OpenGL rendering
* data : a texture object and a display list mapping it on a quad and then
* translating to the right of it.
*
* See the m_charTable member of TextRenderer for an example of use of
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
		bool initialize( QChar c, const QFont &font );
		inline void draw() const
		{
			glCallList( m_displayList );
		}
                inline int height() const { return m_height; }
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
	
	QFontMetrics fontMetrics ( font );
	m_width = fontMetrics.width( c ) + 2;
	m_height = fontMetrics.height() + 2;
	if( m_width <= 2 || m_height <= 2 ) return false;
	QImage image( m_width, m_height, QImage::Format_RGB32 );
	
	QPainter painter;
	painter.begin( &image );
	painter.setFont( font );
	painter.setRenderHint( QPainter::TextAntialiasing );
	painter.setBackground( Qt::black );
	painter.eraseRect( image.rect() );
	painter.setPen( Qt::blue );
	painter.drawText ( 1, m_height - painter.fontMetrics().descent(), c );
	painter.end();

	GLubyte *rawbitmap = new GLubyte[ m_width * m_height ];
	if( ! rawbitmap ) return false;

        int n = 0;
	for( int j = m_height - 1; j >= 0; j-- )
	for( int i = 0; i < m_width; i++, n++ )
	{
		rawbitmap[n] = qBlue( image.pixel( i, j ) );
	}

	float *shadowbitmap = new float[ m_width * m_height ];
	if( ! shadowbitmap ) return false;

        const float df = 0.7;

        for( int j = 1; j < m_height - 1; j++ )
	for( int i = 1; i < m_width - 1; i++ )
        {
          n = i + j * m_width;
          shadowbitmap[n]
            = rawbitmap[n - m_width - 1]*df + rawbitmap[n - m_width] + rawbitmap[n - m_width + 1]*df
            + rawbitmap[n - 1] + rawbitmap[n] + rawbitmap[n + 1]
            + rawbitmap[n + m_width - 1]*df + rawbitmap[n + m_width] + rawbitmap[n + m_width + 1]*df;
        }

        for( int i = 1; i < m_width - 1; i++ )
        {
          n = i;
          shadowbitmap[n]
            = rawbitmap[n - 1] + rawbitmap[n] + rawbitmap[n + 1]
            + rawbitmap[n + m_width - 1]*df + rawbitmap[n + m_width] + rawbitmap[n + m_width + 1]*df;

          n = i + (m_height - 1) * m_width;
          shadowbitmap[n]
            = rawbitmap[n - m_width - 1]*df + rawbitmap[n - m_width] + rawbitmap[n - m_width + 1]*df
            + rawbitmap[n - 1] + rawbitmap[n] + rawbitmap[n + 1];
        }

        for( int j = 1; j < m_height - 1; j++ )
        {
          n = j * m_width;
          shadowbitmap[n]
            = rawbitmap[n - m_width] + rawbitmap[n - m_width + 1]*df
            + rawbitmap[n] + rawbitmap[n + 1]
            + rawbitmap[n + m_width] + rawbitmap[n + m_width + 1]*df;

          n = m_width - 1 + j * m_width;
          shadowbitmap[n]
            = rawbitmap[n - m_width - 1]*df + rawbitmap[n - m_width]
            + rawbitmap[n - 1] + rawbitmap[n]
            + rawbitmap[n + m_width - 1]*df + rawbitmap[n + m_width];
        }

        shadowbitmap[0] = rawbitmap[0] + rawbitmap[1] + rawbitmap[m_width];
        shadowbitmap[m_width-1] = rawbitmap[m_width-1] + rawbitmap[m_width-2] + rawbitmap[2*m_width-1];
        shadowbitmap[(m_height-1)*m_width] = rawbitmap[(m_height-1)*m_width] + rawbitmap[(m_height-2)*m_width] + rawbitmap[(m_height-1)*m_width+1];
        shadowbitmap[(m_height-1)*m_width+m_width-1] = rawbitmap[(m_height-1)*m_width+m_width-1] + rawbitmap[(m_height-1)*m_width+m_width-2] + rawbitmap[(m_height-2)*m_width+m_width-1];

	GLubyte *finalbitmap = new GLubyte[ 2 * m_width * m_height ];
	if( ! finalbitmap ) return false;

        for( int j = 0; j < m_height; j++ )
	for( int i = 0; i < m_width; i++ )
        {
          n = i + j * m_width;
          int luminance, alpha;
          luminance = rawbitmap[n];
          alpha = static_cast<int>(0.5 * shadowbitmap[n]);
          if( alpha > 255 ) alpha = 255;
          finalbitmap[2 * n] = luminance;
          finalbitmap[2 * n + 1] = alpha;
        }

	glGenTextures( 1, &m_texture );
	if( ! m_texture ) return false;

	glBindTexture( GL_TEXTURE_2D, m_texture );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_LUMINANCE_ALPHA,
		m_width,
		m_height,
		0,
		GL_LUMINANCE_ALPHA,
		GL_UNSIGNED_BYTE,
		finalbitmap );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	delete [] rawbitmap;
	delete [] shadowbitmap;
	delete [] finalbitmap;

	m_displayList = glGenLists(1);
	if( ! m_displayList ) return false;

	glNewList( m_displayList, GL_COMPILE );
	glBindTexture( GL_TEXTURE_2D, m_texture );
	glBegin( GL_QUADS );
	glTexCoord2f( 0, 0);
	glVertex2f( 0 , -m_height );
	glTexCoord2f( 1, 0);
	glVertex2f( m_width , -m_height );
	glTexCoord2f( 1, 1);
	glVertex2f( m_width, 0 );
	glTexCoord2f( 0, 1);
	glVertex2f( 0 , 0 );
	glEnd();
	glTranslatef( m_width-2, 0, 0 );
	glEndList();
	return true;
}

TextRenderer::TextRenderer()
{
	m_glwidget = 0;
	m_textmode = false;
}

TextRenderer::~TextRenderer()
{
	QHash<QChar, CharRenderer *>::iterator i = m_charTable.begin();
	while( i != m_charTable.end() )
	{
		delete i.value();
		i = m_charTable.erase(i);
        }
}

void TextRenderer::setGLWidget( GLWidget *glwidget )
{
  m_glwidget = glwidget;
  m_font = m_glwidget->font();
}

void TextRenderer::do_begin()
{
	m_wasEnabled_LIGHTING = glIsEnabled( GL_LIGHTING );
	m_wasEnabled_FOG = glIsEnabled( GL_FOG );
	m_wasEnabled_GL_TEXTURE_2D
		= glIsEnabled( GL_TEXTURE_2D );
	m_wasEnabled_BLEND = glIsEnabled( GL_BLEND );
	glDisable( GL_LIGHTING );
	glDisable( GL_FOG );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, m_glwidget->width(), 0, m_glwidget->height(), 0, 1 );
	glMatrixMode( GL_MODELVIEW );
}

void TextRenderer::begin()
{
	if( ! m_glwidget ) return;
	if( m_textmode ) return;
	m_textmode = true;
	do_begin();
}

void TextRenderer::do_end()
{
	if( ! m_wasEnabled_GL_TEXTURE_2D )
		glDisable( GL_TEXTURE_2D );
	if( ! m_wasEnabled_BLEND ) glDisable( GL_BLEND );
	if( m_wasEnabled_LIGHTING ) glEnable( GL_LIGHTING );
	if( m_wasEnabled_FOG ) glEnable( GL_FOG );
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
}

void TextRenderer::end()
{
	if( m_textmode ) do_end();
	m_textmode = false;
}

int TextRenderer::do_draw( const QString &string )
{
        CharRenderer *c;
        int retval = 0;
	if( m_charTable.contains( string[0] ) )
        {
		c = m_charTable.value( string[0] );
                c->draw();
        }
	else
	{
		c = new CharRenderer;
		if( c->initialize( string[0], m_font ) )
		{
			m_charTable.insert( string[0], c);
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
		if( m_charTable.contains( string[i] ) )
			m_charTable.value( string[i] )->draw();
		else
		{
			CharRenderer *c = new CharRenderer;
			if( c->initialize( string[i], m_font ) )
			{
				m_charTable.insert( string[i], c);
				c->draw();
			}
			else delete c;
		}
	}
        return retval;
}

int TextRenderer::draw( int x, int y, const QString &string )
{
	if( ! m_glwidget ) return 0;
	if( string.isEmpty() ) return 0;

	if( ! m_textmode ) do_begin();
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( x, m_glwidget->height() - y, 0 );
        int retval = do_draw(string);
	glPopMatrix();
	if( ! m_textmode ) do_end();

        return retval;
}

int TextRenderer::draw( const Eigen::Vector3d &pos, const QString &string )
{
	if( ! m_glwidget ) return 0;
	if( string.isEmpty() ) return 0;

        Eigen::Vector3d wincoords = m_glwidget->camera()->project(pos);
	if( ! m_textmode ) do_begin();
	glPushMatrix();
        glLoadIdentity();
	glTranslatef( static_cast<int>(wincoords.x()),
                      static_cast<int>(wincoords.y()),
                      -wincoords.z() );
        int retval = do_draw(string);
	glPopMatrix();
	if( ! m_textmode ) do_end();

        return retval;
}

} // namespace Avogadro
