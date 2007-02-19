/**********************************************************************
  Hydrogens - Hydrogens Plugin for Avogadro

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

#ifndef __HYDROGENS_H
#define __HYDROGENS_H


#include <openbabel/mol.h>

#include <avogadro/glwidget.h>
#include <avogadro/plugin.h>

#include <QObject>
#include <QList>
#include <QString>

namespace Avogadro {

 class Hydrogens : public QObject, public Extension
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::Extension)

    public:
      //! Constructor
      Hydrogens();
      //! Deconstructor
      virtual ~Hydrogens();

      //! \name Description methods
      //@{
      //! Plugin Name (ie Draw)
      virtual QString name() const { return QObject::tr("Hydrogens"); }
      //! Plugin Description (ie. Draws atoms and bonds)
      virtual QString description() const { return QObject::tr("Hydrogens Plugin"); };
      //! Perform Action
      virtual void performAction(QAction *action, Molecule *molecule);
      //@}

    private:
      void addHydrogens(Molecule *molecule);
      void removeHydrogens(Molecule *molecule);
      
  };

} // end namespace Avogadro

#endif
