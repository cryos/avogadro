
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

  This file is part of WmAvo (WiiChem project)
  WmAvo - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmAvo is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmAvo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmAvo. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

/*
  T avoid all warnings from MS SDK, Qt, Avogadro, OpenBabel librairies.
*/
#ifdef _WIN32
#pragma warning( disable : 4251 ) // '...' : class '...' needs to have dll-interface to be used by clients of class '...'
#pragma warning( disable : 4365 ) // conversion from 'x' to 'y', signed/unsigned mismatch
#pragma warning( disable : 4510 ) // '...' : default constructor could not be generated
#pragma warning( disable : 4512 ) // '...' : assignment operator could not be generated
#pragma warning( disable : 4514 ) // '...' : unreferenced inline function has been removed
#pragma warning( disable : 4548 ) // expression before comma has no effect; expected expression with side-effect
#pragma warning( disable : 4571 ) // Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning( disable : 4610 ) //  struct '...' can never be instantiated - user defined constructor required
#pragma warning( disable : 4619 ) //  #pragma warning : there is no warning number '...'
#pragma warning( disable : 4625 ) // '...' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning( disable : 4626 ) // '...' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning( disable : 4628 ) // Digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'. Digraphs are not supported under /Ze. This warning will be followed by an error.
#pragma warning( disable : 4640 ) // '...' : construction of local static object is not thread-safe
#pragma warning( disable : 4668 ) // '...' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#pragma warning( disable : 4710 ) // function not inlined
#pragma warning( disable : 4711 ) // The compiler performed inlining on the given function, although it was not marked for inlining. Inlining is performed at the compiler's discretion. This warning is informational.
#pragma warning( disable : 4714 ) // function '...' // marked as __forceinline not inlined
#pragma warning( disable : 4738 ) // storing 32-bit float result in memory, possible loss of performance
#pragma warning( disable : 4820 ) // 'x' bytes padding added after data member '...'
#pragma warning( push, 0 )
#endif
