/**********************************************************************
  MainWindow.h - main window, menus, main actions

  Copyright (C) 2006-2007 by Geoffrey R. Hutchison
  Some portions Copyright (C) 2006-2007 by Donald E. Curtis

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
      int painterQuality();

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
      void exportPOV();

      void closeFile();

      void cut();
      void copy();
      void paste();
      void clear();
      void selectAll();
      void selectNone();

      void newView();
      void closeView();
      void centerView();
      void setView(int index);
      void fullScreen();
      void showSettingsDialog();
      void setBackgroundColor();
      void setPainterQuality(int quality);

      void undoStackClean(bool clean);

      void clearRecentFiles();
      void about();

      void actionTriggered();
      void documentWasModified();

      void setTool(Tool *tool);

      // Mac helper
      void macQuit();

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

      //! Find an existing main window by filename
      //! \return a pointer to the appropriate window, or NULL if none exists
      MainWindow* findMainWindow(const QString &fileName);

      void setMolecule(Molecule *molecule);
      Molecule* molecule() const;

      void loadExtensions();

      //! Helper function for cut or copy -- prepare a clipboard
      QMimeData* prepareClipboardData(QList<Primitive*> selectedItems);

      //! Helper function for Mac -- hide main window and disable menus
      void hideMainWindowMac();
      //! Helper function for Mac -- show main window and re-enable menus
      void showMainWindowMac();
  };

  /**
   * \mainpage v0.1.0 API Documentation
   *
   * \section avogadro Introduction
   *
   * Avogadro is a molecular modeling / viewing / editing tool.
   * The design idea behind Avogadro is to allow every feature to be
   * pluggable.  This allows new features and tools to be easily added
   * and removed and also provides a solid framework for research.
   *
   *
   * \subsection main Main Classes
   *
   * libavogadro interfaces:
   * - Engine : Interface for engine plugins
   * - Tool : Interface for tool plugins
   *
   * libavogadro classes:
   * - GLWidget : Widget for rendering 3d representations of a molecule.
   * - MoleculeTreeView : Widget for rendering a tree view representation of a molecule.
   * - Primitive : Base class for all model components
   *   - Atom : Class for representing atoms
   *   - Bond : Class for representing bonds
   *   - Residue : Class for representing residues
   *   - Molecule : Class for representing molecules
   *
   *
   * avogadro interfaces:
   * - Extension : Interface for extension plugins
   *
   * avogadro classes:
   * - n/a
   *
   */

} // end namespace Avogadro
#endif
