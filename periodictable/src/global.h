/**********************************************************************
  periodictableexport.h - Setup some default defines.

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

#ifndef PERIODICTABLEEXPORT_H
#define PERIODICTABLEEXPORT_H

#ifdef WIN32
# ifndef NOMINMAX
#  define NOMINMAX 1
#  include <limits>
# endif
#endif

// If we are using a recent GCC version with visibility support use it
// The Krazy checker doesn't like this, but it's a valid CMake conditional
// krazy:excludeall=cpp
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

#endif  // PERIODICTABLEEXPORT_H

