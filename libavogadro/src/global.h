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

#define ROTATION_SPEED     0.005
#define ZOOM_SPEED         0.02
#define MOUSE_WHEEL_SPEED  0.1
#define CAMERA_NEAR_DISTANCE     2.0
#define CAMERA_MOL_RADIUS_MARGIN 10.0
#define SEL_BUF_MAX_SIZE         262144
#define SEL_BUF_MARGIN_NEW_ATOMS 256

#endif  // __GLOBAL_H
