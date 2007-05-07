/**********************************************************************
  global.h - Setup some default defines.

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Copyright (C) 2006,2007 by Donald Ephraim Curtis

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

#ifndef __GLOBAL_H
#define __GLOBAL_H

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
const int      PAINTER_GLOBAL_QUALITY_SETTINGS       = 3;
const int      DEFAULT_GLOBAL_QUALITY_SETTING        = PAINTER_GLOBAL_QUALITY_SETTINGS - 1;
const int      PAINTER_DETAIL_LEVELS                 = 10;
const int      PAINTER_SPHERES_LEVELS_ARRAY[3][10]
                 = { {0, 0, 1, 1, 2, 2, 3, 3, 4, 4} ,
                     {0, 1, 2, 3, 4, 4, 5, 5, 6, 6} ,
                     {0, 2, 3, 4, 5, 6, 7, 8, 9, 9} };
const double   PAINTER_SPHERES_LIMIT_MIN_LEVEL       = 0.005;
const double   PAINTER_SPHERES_LIMIT_MAX_LEVEL       = 0.15;

const int      PAINTER_CYLINDERS_LEVELS_ARRAY[3][10]
                 = { {0, 3, 5, 5, 8, 8, 12, 12, 16, 16} ,
                     {0, 4, 6, 9, 12, 12, 16, 16, 20, 20},
                     {0, 4, 6, 10, 14, 18, 22, 26, 32, 40} };
const double   PAINTER_CYLINDERS_LIMIT_MIN_LEVEL       = 0.001;
const double   PAINTER_CYLINDERS_LIMIT_MAX_LEVEL       = 0.03;

#endif  // __GLOBAL_H
