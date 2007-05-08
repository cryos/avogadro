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

#ifndef __TEXTRENDERER_H
#define __TEXTRENDERER_H

#include<QHash>
#include<QFont>
#include<eigen/vector.h>
#include<avogadro/global.h>
#include<QGLWidget>

/** NOTE: This class is only there as a temporary replacement for
* QGLWidget::renderText(). As of Qt 4.2.3 and Qt 4.3-beta1, this function is
* too slow and doesn't allow outlined text. If a future version of Qt brings a
* sufficiently improved QGLWidget::renderText(), we will of course drop this class.
*
* This class renders text inside a QGLWidget. It replaces the functionality
* of QGLWidget::renderText().
*
* Its advantages over the renderText() in Qt 4.2.3 are that it is much faster,
* consumes far less memory, does outlined text, and provides a mode (the 2D mode) where
* it is guaranteed that text gets the lowest value in the Z-buffer. Its drawbacks are that
* it can't handle properly multiple fonts, and that its Unicode-safeness is
* not perfect. More precisely, it does not support combinations of unicode characters.
* For example, for accented characters, there are two possible unicode representations:
* either use a single unicode character representing the accented letter, or use the unicode
* character of the non-accented letter and combine it with the unicode character of the accent.
* This TextRenderer class does not support this second approach. So use the unicode character
* representing the accented letter. I don't think this is a big deal: there exist unicode characters
* for pretty much any accented letter. And anyway most of the text we're going to render is
* numbers and other scientific data, not medieval poetry.
*
* Any QFont can be used, all character encodings supported by Qt can be used. Before painting any
* text, and after the OpenGL context is created, you should set the GLWidget where to render to by
* calling setGLWidget().
*
* To print text, do:
* @code
	textRenderer.print( x, y, string );
* @endcode
* where x,y are ints and string is any QString. If you want to choose a color,
* please call glColor3f or glColor4f before calling print(). Of course you can
* also call qglColor or Color::apply. You can achieve semitransparent text at
* no additional cost by choosing a semitransparent color.
*
* If you wish to do several calls to print(), it will improve performance
* to enclose them between a call to begin() and a call to end(), like that:
* @code
	textRenderer.begin();
	textRenderer.print( x1, y1, string1 );
	textRenderer.print( x2, y2, string2 );
	textRenderer.print( x3, y2, string3 );
	textRenderer.end();
* @endcode
* 
* Please make sure, though, that no relevant OpenGL state change occurs between
* begin() and end(), except the state changes performed by the TextRenderer
* itself. In other words, please avoid calling glSomething() between begin() and
* end(), except if you are sure that this call won't result in a conflicting state
* change.
*
* The print() method when called alone, or the begin()-print()-end() group,
* do restore the OpenGL state as they found it, including the matrices.
*
* If you experience rendering problems, you can try the following:
* - disable some OpenGL state bits. For instance, TextRenderer automatically
*   disables fog and lighting during rendering, because it doesn't work
*   correctly with them enabled. There probably are other OpenGL state bits
*   that have to be disabled, so if your program enables some of them, you
*   might have to disable them before rendering text.
* - if you experience poor font quality, please consider using an antialiased
*   font.
*
* @class TextRenderer
* @author Benoit Jacob
*/

namespace Avogadro {
class CharRenderer;
class A_EXPORT GLWidget;
class TextRenderer
{
	protected:
		/**
		 * The font used for rendering the chars.
		 */
		QFont m_font;
		
		/**
		 * This hash gives the correspondence table between QChars
		 * (the keys) and the corresponding CharRenderers (the values).
		 * Every time a QChar is being met, either it is found in this
		 * table, in which case it can be directly rendered, or it is
		 * not found, in which case a new CharRenderer is created for
		 * it and added to this table.
		 */
		QHash<QChar, CharRenderer*> m_charTable;

		/**
		 * The GLWidget in which to render. This is set
		 * once and for all by setup().
		 */
                GLWidget *m_glwidget;

		GLboolean m_textmode;

		///{ Members used to remember the OpenGL state in order to be able to restore it after rendering. See do_end().
		GLboolean m_wasEnabled_LIGHTING;
		GLboolean m_wasEnabled_GL_TEXTURE_2D;
		GLboolean m_wasEnabled_FOG;
		GLboolean m_wasEnabled_BLEND;
		///}

		/**
		 * Stores the relevant part of the OpenGL state, and prepares
		 * for rendering
		 */
		void do_begin();

		/**
		 * Restores the OpenGL state
		 */
		void do_end();

    int do_draw(const QString &string);

	public:
		TextRenderer();
		~TextRenderer();
		
		/**
		 * This should be called only once, before any printing occurs.
		 * @param glwidget The GLWidget in which to render.
		 * See m_glwidget member.
		 * @param font The QFont to use. See m_font member.
		 */
		void setGLWidget( GLWidget *glwidget );

		/**
		 * Prints text at the position (x,y) in window coordinates
		 * (0,0) is the bottom left corner
		 * @param x the x-coordinate
		 * @param y the y-coordinate
		 * @param string the QString to print
		 */
		int draw( int x, int y, const QString &string);

		/**
		 * Call this before doing multiple calls to print(). This is
		 * not necessary, but will improve performance. Don't forget,
		 * then, to call end() after.
		 */
		void begin();

		/**
		 * Call this after having called begin() and print().
		 */
		void end();

    int draw( const Eigen::Vector3d & pos, const QString &string);
};

} // namespace Avogadro

#endif // __TEXTRENDERER_H
