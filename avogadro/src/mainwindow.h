/**********************************************************************
  MainWindow.h - main window, menus, main actions

  Copyright (C) 2006 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006 by Donald E. Curtis

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

#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include "ui_mainwindow.h"
#include "flowlayout.h"
#include "flattabwidget.h"

#include <QMainWindow>

class QUndoStack;
class QStackedLayout;
class QStandardItem;

namespace Avogadro {

  class MainWindowPrivate;
  class MainWindow : public QMainWindow
  {
    Q_OBJECT;

    public:
      MainWindow();
      MainWindow(const QString &fileName);

      bool loadFile(const QString &fileName);
      bool saveFile(const QString &fileName);

    protected:
      void closeEvent(QCloseEvent *event);

    public Q_SLOTS:
      void newFile();
      void openFile();
      void openFile(const QString &fileName);
      void openRecentFile();
      bool save();
      bool saveAs();
      void revert();
      void exportGraphics();

      void closeFile();

      void cut();
      void copy();
      void paste();
      void clear();

      void newView();
      void closeView();
      void centerView();
      void setView(int index);
      void fullScreen();
      void setBackgroundColor();

      void undoStackClean(bool clean);

      void clearRecentFiles();
      void about();

      void actionTriggered();
      void documentWasModified();

      void setTool(Tool *tool);

//       void updateEngine( QStandardItem *item );

    private:
      friend class MainWindowPrivate;
      MainWindowPrivate * const d;

      Ui::MainWindow ui;

      enum { maxRecentFiles = 10 };

      void constructor();
      void readSettings();
      void writeSettings();
      void connectUi();
      
      bool maybeSave();
      void setFileName(const QString &fileName);
      void updateRecentFileActions();
      QString strippedName(const QString &fullFileName);
      //! Find an existing main window by filename
      //! \return a pointer to the appropriate window, or NULL if none exists
      MainWindow* findMainWindow(const QString &fileName);

      void setMolecule(Molecule *molecule);
      Molecule* molecule() const;

      void loadExtensions();
      
      //! Helper function for cut or copy -- prepare a clipboard
      QMimeData* prepareClipboardData(); 
  };

  /**
   * \mainpage v0.0.3 API Documentation
   *
   * \section avogadro Introduction
   *
   * Avogadro is a molecular modeling / viewing / editing tool.
   * The design idea behind Avogadro is to allow every feature to be 
   * pluggable.  This allows new features and tools to be easily added
   * and removed and also provides a solid framework for research.
   *
   * - \ref main "Main Classes"
   *
   * \page main "Main Classes"
   *
   * libavogadro interfaces:
   * - Engine Interface for engine plugins
   *
   * libavogadro classes:
   * - GLWidget Widget for rendering 3d representations of a molecule.
   * - MoleculeTreeView Widget for rendering a tree view representation of a molecule.
   * - Primitive Base class for all model components
   *   - Atom Class for representing atoms
   *   - Bond Class for representing bonds
   *   - Residue Class for representing residues
   *   - Molecule Class for representing molecules
   *
   *
   * avogadro classes:
   * - n/a
   *
   */

} // end namespace Avogadro
#endif
