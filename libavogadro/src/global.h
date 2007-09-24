/**********************************************************************
  global.h - Setup some default defines.

  Copyright (C) 2007 by Donald Ephraim Curtis

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

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <QString>

#ifdef WIN32
# ifndef NOMINMAX
#  define NOMINMAX 1
# endif
#endif

#ifdef WIN32
# define A_DECL_IMPORT __declspec(dllimport)
# define A_DECL_EXPORT __declspec(dllexport)
#else
# define A_DECL_IMPORT
# define A_DECL_EXPORT
#endif

#ifndef A_EXPORT
# ifdef avogadro_lib_EXPORTS
#  define A_EXPORT A_DECL_EXPORT
# else
#  define A_EXPORT A_DECL_IMPORT
# endif
#else
# define A_EXPORT
#endif

#ifndef GL_RESCALE_NORMAL
# define GL_RESCALE_NORMAL 0x803A
#endif

#ifndef GL_COLOR_SUM_EXT
# define GL_COLOR_SUM_EXT 0x8458
#endif

#ifndef GL_LIGHT_MODEL_COLOR_CONTROL_EXT
# define GL_LIGHT_MODEL_COLOR_CONTROL_EXT 0x81F8
#endif

#ifndef GL_SEPARATE_SPECULAR_COLOR_EXT
# define GL_SEPARATE_SPECULAR_COLOR_EXT 0x81FA
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
# define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif

const double   ROTATION_SPEED                        = 0.005;
const double   ZOOM_SPEED                            = 0.02;
const double   MOUSE_WHEEL_SPEED                     = 0.1;
const double   CAMERA_MOL_RADIUS_MARGIN              = 10.0;
const double   CAMERA_NEAR_DISTANCE                  = 2.0;
const int      SEL_BUF_MAX_SIZE                      = 262144;
const int      SEL_BUF_MARGIN                        = 128;
const int      SEL_BOX_HALF_SIZE                     = 4;
const int      SEL_BOX_SIZE                          = 2 * SEL_BOX_HALF_SIZE + 1;
const double   SEL_ATOM_EXTRA_RADIUS                 = 0.18;
const double   SEL_BOND_EXTRA_RADIUS                 = 0.07;

namespace Avogadro
{
  QString A_EXPORT libVersion();
  QString A_EXPORT libSvnRevision();
  QString A_EXPORT libPrefix();
}

#endif  // __GLOBAL_H
