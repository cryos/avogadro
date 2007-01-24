/**********************************************************************
  Ghemical - LibGhemical Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

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

#ifndef __GHEMICAL_H
#define __GHEMICAL_H


#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include <avogadro/plugin.h>

namespace Avogadro {

 class Ghemical : public QObject, public Plugin
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::Plugin)

    public:
      //! Constructor
      Ghemical();
      //! Deconstructor
      virtual ~Ghemical();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Ghemical"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("LibGhemical Plugin"); };
      //@}
      
    public slots:
      //! \name Plugin Methods
      //@{
      //! \brief Callback methods for ui.actions on the canvas.
      //! Random calls.
      //@}
  };

} // end namespace Avogadro

#endif
