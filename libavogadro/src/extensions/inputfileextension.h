/**********************************************************************
  InputFileExtension - Extension for generating input files

  Copyright (C) 2008 Marcus D. Hanwell

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

#ifndef INPUTFILEEXTENSION_H
#define INPUTFILEEXTENSION_H

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include "daltoninputdialog.h"
#include "gaussianinputdialog.h"
#include "molproinputdialog.h"
#include "mopacinputdialog.h"
#include "nwcheminputdialog.h"
#include "qcheminputdialog.h"


namespace Avogadro
{
  class InputFileExtension : public Extension
  {
  Q_OBJECT
    AVOGADRO_EXTENSION("Input File Generators", tr("Input File Generators"), 
                       tr("Create input files for quantum chemistry packages"))
  public:
    InputFileExtension(QObject* parent = 0);
    virtual ~InputFileExtension();

    virtual QList<QAction *> actions() const;

    virtual QString menuPath(QAction* action) const;

    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    void setMolecule(Molecule *molecule);

    /**
     * Save the settings for this extension.
     * @param settings Settings variable to write settings to.
     */
    void writeSettings(QSettings &settings) const;

    /**
     * Read the settings for this extension.
     * @param settings Settings variable to read settings from.
     */
    void readSettings(QSettings &settings);
    
  public Q_SLOTS:
    void readOutputFile(const QString filename);

  private:
    DaltonInputDialog* m_daltonInputDialog;
    GaussianInputDialog* m_gaussianInputDialog;
    MolproInputDialog* m_molproInputDialog;
    MOPACInputDialog* m_mopacInputDialog;
    NWChemInputDialog* m_nwchemInputDialog;
    QChemInputDialog* m_qchemInputDialog;

    QList<QAction *> m_actions;
    Molecule *m_molecule;
    
    GLWidget *m_widget;

  };

  class InputFileExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(InputFileExtension) 
  };

} // End namespace Avogadro

#endif
