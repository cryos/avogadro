/**********************************************************************
  GLWidget - OpenGL selection handling

  Copyright (C) 2006,2007 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
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

#ifndef __GLPICKER_H
#define __GLPICKER_H

#include <QGLWidget>
#include <avogadro/global.h>

namespace Avogadro {
  
  class GLPicker
  {
    public:
      GLPicker(const GLwidget *widget);
      ~GLPicker();
      
      /** Finds the list of all primitives in a
        * rectangular region of the GLWidget */
      QList<Primitive *> primitives (const QRect &rect);
      
      /** Finds the nearest primitive under the specified pixel of the GLWidget.
        * If no primitive is found under this pixel, the 0 pointer is returned.
        */
      Primitive *primitive (const QPoint &pos);
      
      /** Finds the nearest atom under the specified pixel of the GLWidget.
        * If no atom is found under this pixel, the 0 pointer is returned. */
      Atom *atom (const QPoint &pos);
      
      /** Finds the nearest bond under the specified pixel of the GLWidget.
        * If no bond is found under this pixel, the 0 pointer is returned. */
      Bond *bond (const QPoint &pos);
      
      /** Finds the position in 3-space of the atom under the specified
        * pixel of the GLWidget. This function handles unitcells correctly,
        * allowing to pick a particular atom in a crystal.
        *
        * @returns true if an atom was found under the specified pixel;
        *          false otherwise
        * @param pos the specified pixel
        * @param res pointer to where to store the result
        */
      bool findAtomPos(const QPoint &pos, Eigen::Vector3d *res);

    private:
      GLPickerPrivate * const d;
  
  };

} // end namespace avogadro

#endif // __GLPICKER_H
