/**********************************************************************
  DipoleEngine - Engine to display a 3D vector such as the dipole moment

  Copyright (C) 2008      Geoffrey R. Hutchison

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

#ifndef DIPOLEENGINE_H
#define DIPOLEENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

namespace Avogadro {

  //! Dipole Engine class.
  class DipoleEngine : public Engine
  {
    Q_OBJECT
    AVOGADRO_ENGINE(tr("Dipole"))

    public:
      //! Constructor
      DipoleEngine(QObject *parent=0);
      //! Deconstructor
      ~DipoleEngine();

      //! Copy
      Engine *clone() const;

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      //@}

      double transparencyDepth() const;
      EngineFlags flags() const;

      double radius(const PainterDevice *pd, const Primitive *p = 0) const;

  };

  //! Generates instances of our AxesEngine class
  class DipoleEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)
    AVOGADRO_ENGINE_FACTORY(DipoleEngine)
  };

} // end namespace Avogadro

#endif
