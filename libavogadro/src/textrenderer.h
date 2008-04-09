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

#ifndef __TEXTRENDERER_H
#define __TEXTRENDERER_H

#include <avogadro/global.h>
#include <eigen/vector.h>


/**
 * @class TextRenderer
 * @internal
 * @author Benoit Jacob
 *
 * NOTE: This class is only there as a temporary replacement for
 * QGLWidget::renderText(). As of Qt 4.2.3 and Qt 4.3-beta1, this function is
 * too slow and can't do outlined text. If a future version of Qt brings a
 * sufficiently improved QGLWidget::renderText(), we will of course drop this class.
 *
 * This class renders text inside a QGLWidget. It replaces the functionality
 * of QGLWidget::renderText().
 *
 * Its advantages over the renderText() in Qt 4.2.3 are that it is much faster both at
 * render-time and at startup, consumes less memory, and does outlined text. Its drawbacks are that
 * it can't yet handle rendering more than one font simultaneously, and that its Unicode-safeness is
 * not perfect as superpositions of unicode characters aren't handled.
 *
 * Every QFont can be used, every character encodings supported by Qt can be used.
 *
 * To draw plain 2D text on top of the scene, do:
 * @code
 textRenderer.begin();
 textRenderer.draw( x1, y1, string1 );
 textRenderer.draw( x2, y2, string2 );
 textRenderer.draw( x3, y2, string3 );
 textRenderer.end();
 * @endcode
 *
 * To draw text as a transparent object inside the scene, do:
 * @code
 textRenderer.begin();
 textRenderer.draw( pos1, string1 );
 textRenderer.draw( pos2, string2 );
 textRenderer.draw( pos3, string3 );
 textRenderer.end();
 * @endcode
 *
 * In order to set the text color, please call glColor3f or glColor4f before
 * calling draw(). Of course you can
 * also call qglColor or Color::apply(). You can achieve semitransparent text at
 * no additional cost by choosing a semitransparent color.
 *
 * Please make sure that no relevant OpenGL state change occurs between
 * begin() and end(), except the state changes performed by the TextRenderer
 * itself. In other words, please avoid calling glSomething() between begin() and
 * end(), except if you are sure that this call won't result in a conflicting state
 * change. Of course calling glColor*() is allowed.
 *
 * If you experience rendering problems, you can try the following:
 * - disable some OpenGL state bits. For instance, TextRenderer automatically
 *   disables fog and lighting during rendering, because it doesn't work
 *   correctly with them enabled. There probably are other OpenGL state bits
 *   that have to be disabled, so if your program enables some of them, you
 *   might have to disable them before rendering text.
 * - if you experience poor font quality, meake sure that your GLWidget is using
 *   an antialiased font.
 *
 */

namespace Avogadro
{

  class CharRenderer;
  class GLWidget;

  class TextRendererPrivate;
  class TextRenderer
  {
    public:
      TextRenderer();
      ~TextRenderer();

      /**
       * This should be called only once, before any printing occurs.
       * @param glwidget The GLWidget in which to render.
       */
//       void setGLWidget( GLWidget *glwidget );

      /**
       * Call this before drawing any text. This method saves the GL state
       * and changes it to prepare for text rendering.
       */
      void begin(GLWidget *widget);

      /**
       * Call this after drawing text. This method restores the GL state
       * to what it was when begin() was called.
       */
      void end();

      /**
       * Draw text inside the 3D scene. Must be called between begin() and end().
       * The text is centered (both horizontally and vertically) around the specified position.
       * @param pos the position of the text in the scene's coordinate system
       * @param string the QString to render
       * @returns the height in pixels of the text just rendered (0 for an empty string).
       */
      int draw( const Eigen::Vector3d & pos, const QString &string);

      /**
       * Draw 2D text at the position (x,y) in window coordinates. Must be called
       * between begin() and end().
       * (0,0) is the top-left corner.
       * @param x the x-coordinate
       * @param y the y-coordinate
       * @param string the QString to render
       * @returns the height in pixels of the text just rendered (0 for an empty string).
       */
      int draw( int x, int y, const QString &string);

      bool isActive();

    private:

      TextRendererPrivate * const d;
  };

} // namespace Avogadro

#endif // __TEXTRENDERER_H
