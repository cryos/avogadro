/**********************************************************************
  ForceEngine - Display forces

  Copyright (C) 2008 by Tim Vandermeersch

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

#ifndef FORCEENGINE_H
#define FORCEENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

namespace Avogadro {

  //! ForceEngine class.
  //class ForceSettingsWidget;
  class ForceEngine : public Engine
  {
    Q_OBJECT
      AVOGADRO_ENGINE("Force", tr("Force"))

    public:
      //! Constructor
      ForceEngine(QObject *parent=0);

      Engine *clone() const;

      //! Deconstructor
      ~ForceEngine();

      //! \name Render Methods
      //@{
      bool renderOpaque(PainterDevice *pd);
      bool renderOpaque(PainterDevice *pd, const Atom *a);
      //@}
  };

  //! Generates instances of our ForceEngine class
  class ForceEngineFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_ENGINE_FACTORY(ForceEngine, tr("Renders force displacements on atoms"))
  };

} // end namespace Avogadro

#endif
