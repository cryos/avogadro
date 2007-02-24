/**********************************************************************
  Ghemical - Ghemical Plugin for Avogadro

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
#include <openbabel/forcefield.h>

#include <avogadro/glwidget.h>
#include <avogadro/plugin.h>

#include <QObject>
#include <QList>
#include <QString>

#ifndef BUFF_SIZE
#define BUFF_SIZE 256
#endif

namespace Avogadro {

 class Ghemical : public QObject, public Extension
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::Extension)

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
      virtual QString description() const { return QObject::tr("Ghemical Plugin"); };
      //! Perform Action
      virtual void performAction(QAction *action, Molecule *molecule, QTextEdit *messages);
      //@}

    private:
      void optimize(Molecule *molecule, QTextEdit *messages);
      
      OpenBabel::OBForceField* pGhemicalFF;
  };

} // end namespace Avogadro

#endif
