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

#include <QMainWindow>
#include <QtGui>

#include "GLWidget.h"
#include "Renderer.h"

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

namespace Avogadro {

  class MainWindow : public QMainWindow
  {
    Q_OBJECT

    public:
      MainWindow();
      MainWindow(const QString &fileName);
      ~MainWindow();

      bool loadFile(const QString &fileName);
      bool saveFile(const QString &fileName);

      Molecule *getMolecule() { return(&molecule); }
      Renderer *defaultRenderer;

    protected:
      void closeEvent(QCloseEvent *event);

    public slots:
      void newFile();
      void open();
      void openRecentFile();
      bool save();
      bool saveAs();
      void revert();
      void exportGraphics();

      void undo();
      void redo();

      void clearRecentFiles();
      void about();

      void documentWasModified();

    private:
      GLWidget  *gl;
      Molecule molecule;
      OpenBabel::OBMol view;
      QString    currentFile;
      bool       isModified;

      QMenu      *menuFile;
      QMenu      *menuOpen_Recent;
      QMenu      *menuEdit;
      QMenu      *menuHelp;
      QToolBar   *toolBar;

      QAction    *actionQuit;

      QAction    *actionNew;
      QAction    *actionOpen;
      QAction    *actionClose;
      QAction    *actionSave;
      QAction    *actionSaveAs;
      QAction    *actionRevert;
      QAction    *actionExport;

      enum { maxRecentFiles = 5 };
      QAction    *actionRecentFile[maxRecentFiles];
      QAction    *actionClearRecentMenu;
      QAction    *actionSeparator;

      QAction    *actionUndo;
      QAction    *actionRedo;

      QAction    *actionAbout;

      void init();
      void readSettings();
      void writeSettings();
      void createActions();
      void createMenuBar();
      void createToolbars();
      bool maybeSave();
      void setCurrentFile(const QString &fileName);
      void updateRecentFileActions();
      QString strippedName(const QString &fullFileName);

      void loadRenderers();

      MainWindow *findMainWindow(const QString &fileName);
  };

} // end namespace Avogadro
#endif
