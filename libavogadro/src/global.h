/**********************************************************************
  global.h - Setup some default defines.

  Copyright (C) 2007 by Donald Ephraim Curtis
  Copyright (C) 2008-2009 by Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QTranslator>

#define EIGEN_WORK_AROUND_QT_BUG_CALLING_WRONG_OPERATOR_NEW_FIXED_IN_QT_4_5

#ifdef WIN32
# ifndef NOMINMAX
#  define NOMINMAX 1
#  include <limits>
# endif
#endif

// If we are using a recent GCC version with visibility support use it
#ifdef HAVE_GCC_VISIBILITY
  #define A_DECL_IMPORT __attribute__ ((visibility("default")))
  #define A_DECL_EXPORT __attribute__ ((visibility("default")))
  #define A_DECL_HIDDEN __attribute__ ((visibility("hidden")))
#elif defined(WIN32)
  #define A_DECL_IMPORT __declspec(dllimport)
  #define A_DECL_EXPORT __declspec(dllexport)
  #define A_DECL_HIDDEN
#else
  #define A_DECL_IMPORT
  #define A_DECL_EXPORT
  #define A_DECL_HIDDEN
#endif

// This macro should be used to export parts of the API
#ifndef A_EXPORT
  #ifdef avogadro_EXPORTS
    #define A_EXPORT A_DECL_EXPORT
  #else
    #define A_EXPORT A_DECL_IMPORT
  #endif
#endif

// This macro allows the selective hiding of parts of our exposed API
#ifndef A_HIDE
  #define A_HIDE A_DECL_HIDDEN
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

namespace Avogadro
{
  const double   ROTATION_SPEED                    = 0.005;
  const double   ZOOM_SPEED                        = 0.02;
  const double   MOUSE_WHEEL_SPEED                 = 0.1;
  const double   CAMERA_MOL_RADIUS_MARGIN          = 10.0;
  const double   CAMERA_NEAR_DISTANCE              = 2.0;
  const int      SEL_BUF_MAX_SIZE                  = 262144;
  const int      SEL_BUF_MARGIN                    = 128;
  const int      SEL_BOX_HALF_SIZE                 = 4;
  const int      SEL_BOX_SIZE                      = 2 * SEL_BOX_HALF_SIZE + 1;
  const double   SEL_ATOM_EXTRA_RADIUS             = 0.18;
  const double   SEL_BOND_EXTRA_RADIUS             = 0.07;

  const float    LIGHT_AMBIENT[4]                  = {0.2f, 0.2f, 0.2f, 1.0f};

  const float    LIGHT0_DIFFUSE[4]                 = {1.0f, 1.0f, 1.0f, 1.0f};
  const float    LIGHT0_SPECULAR[4]                = {1.0f, 1.0f, 1.0f, 1.0f};
  const float    LIGHT0_POSITION[4]                = {0.8f, 0.7f, 1.0f, 0.0f};

  const float    LIGHT1_DIFFUSE[4]                 = {0.3f, 0.3f, 0.3f, 1.0f};
  const float    LIGHT1_SPECULAR[4]                = {0.5f, 0.5f, 0.5f, 1.0f};
  const float    LIGHT1_POSITION[4]                = {-0.8f, 0.7f, -0.5f, 0.0f};

  const double   cPi                               = 3.14159265358979323846;
  const double   cDegToRad                         = cPi / 180.0;

  /**
   * @class Library global.h <avogadro/global.h>
   * @brief Interface for static calls for versions, installation prefix, and
   * other compiler macros.
   * @author Marcus D. Hanwell
   */
  class A_EXPORT Library
  {
    public:
      /**
       * Create a translator instance for the Avogadro library.
       */
      static QTranslator *createTranslator();

      /**
       * @return The version of the Avogadro library.
       */
      static QString version();

      /**
       * The source control management revision.
       */
      static QString scmRevision();

      /**
       * The installation prefix that the Avogadro library was compiled into.
       * Typically /usr/local or /usr
       */
      static QString prefix();

      /**
       * @return True if mutlithreaded OpenGL rendering is enabled.
       */
      static bool threadedGL();
  };
}

#endif  // __GLOBAL_H

