/**********************************************************************
  FileImportExtension - Extension for importing files

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

#ifndef FILEIMPORTEXTENSION_H
#define FILEIMPORTEXTENSION_H

#include "ui_fileimportdialog.h"

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

class QTextStream;

namespace Avogadro
{

  class FileImportDialog;

  class FileImportExtension : public Extension
  {
  Q_OBJECT

  public:
    FileImportExtension(QObject* parent = 0);
    virtual ~FileImportExtension();

    virtual QString name() const { return QObject::tr("Import Trajectory"); }
    virtual QString description() const
    {
      return QObject::tr("Import chemical files");
    }

    /**
     * @return a list of actions which this widget can perform
     */
    virtual QList<QAction *> actions() const;

    /**
     * @return the menu path for the specified action
     */
    virtual QString menuPath(QAction* action) const;

    /**
     * @param action The action that triggered the calls.
     * @param widget The currently active GLWidget (feedback to the user).
     * @return An undo command for this action.
     */
    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    /**
     * Save the settings for this extension.
     * @param settings Settings variable to write settings to.
     */
    virtual void writeSettings(QSettings &settings) const;

    /**
     * Read the settings for this extension.
     * @param settings Settings variable to read settings from.
     */
    virtual void readSettings(QSettings &settings);

  public Q_SLOTS:

    /**
     * Slot to change the current molecule.
     */
    void setMolecule(Molecule *molecule);

  private:
    GLWidget* m_glwidget;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    FileImportDialog *m_fileImportDialog;

    void readParmFile(Molecule *mol);
    void processLine(QTextStream *in, Molecule *mol);

  private Q_SLOTS:
    void import();
    void selectFileName();
    void selectFileName2();

  };

  class FileImportDialog : public QDialog, public Ui::FileImportDialog
  {
    public:
      FileImportDialog(QDialog *parent=0) : QDialog(parent) {
        setupUi(this);
      }
  };

  class FileImportExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(FileImportExtension,
        tr("File Import Extension"),
        tr("Extension for importing chemical file format files."))

  };

} // End namespace Avogadro

#endif
