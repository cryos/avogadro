/**********************************************************************
  OrbitalExtension - Extension for generating orbital cubes

  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef ORBITALEXTENSION_H
#define ORBITALEXTENSION_H

#include "basisset.h"
#include "orbitaldialog.h"

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

namespace Avogadro
{
  class OrbitalExtension : public Extension
  {
  Q_OBJECT

  public:
    OrbitalExtension(QObject* parent = 0);
    virtual ~OrbitalExtension();

    virtual QString name() const { return QObject::tr("Orbitals"); }
    virtual QString description() const
    {
      return QObject::tr("Orbital cube generator");
    }

    virtual QList<QAction *> actions() const;

    virtual QString menuPath(QAction* action) const;

    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    void setMolecule(Molecule *molecule);

  private:
    GLWidget* m_glwidget;
    OrbitalDialog* m_orbitalDialog;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    BasisSet* m_basis;

  private Q_SLOTS:
    void loadBasis(QString fileName);
    void calculateMO(int n);

  };

  class OrbitalExtensionFactory : public QObject, public PluginFactory
  {
      Q_OBJECT
      Q_INTERFACES(Avogadro::PluginFactory)

    public:
      Plugin *createInstance(QObject *parent = 0) { return new OrbitalExtension(parent); }
      int type() const { return Plugin::ExtensionType; };
      QString name() const { return tr("Orbital Extension"); };
      QString description() const { return tr("Extension for calculating orbitals."); };
 
  };

} // End namespace Avogadro

#endif
