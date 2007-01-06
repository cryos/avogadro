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
#include <QObject>
#include <QDockWidget>

#include <avogadro/glwidget.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include "tool.h"
#include "projectmodel.h"

namespace Avogadro {

  class MainWindow : public QMainWindow
  {
    Q_OBJECT

    public:
      MainWindow();
      MainWindow(const QString &fileName);

      bool loadFile(const QString &fileName);
      bool saveFile(const QString &fileName);

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

      void fullScreen();
      void setBackgroundColor();

      void clearRecentFiles();
      void about();

      void documentWasModified();

      void setCurrentTool(Tool *tool);
      void setCurrentTool(int i);

      void glMousePress(QMouseEvent *event);
      void glMouseMove(QMouseEvent *event);
      void glMouseRelease(QMouseEvent *event);

    private:
      GLWidget  *gl;
      ProjectModel *model;
      Tool      *currentTool;
      QList<Tool *> tools;

      QString    currentFile;
      bool       isModified;
      QUndoStack *undo;

      //QGridLayout *layout;
      QDockWidget *dockTools;
      QDockWidget *dockToolProperties;
      QDockWidget *dockProject;
      QTreeView *treeProject;


      QMenu      *menuFile;
      QMenu      *menuOpen_Recent;
      QMenu      *menuEdit;
      QMenu      *menuView;
      QMenu      *menuSettings;
      QMenu      *menuSettingsToolbars;
      QMenu      *menuSettingsDocks;
      QMenu      *menuHelp;
      QToolBar   *tbFile;

      QAction    *actionQuit;

      QAction    *actionNew;
      QAction    *actionOpen;
      QAction    *actionClose;
      QAction    *actionSave;
      QAction    *actionSaveAs;
      QAction    *actionRevert;
      QAction    *actionExport;

      enum { maxRecentFiles = 10 };
      QAction    *actionRecentFile[maxRecentFiles];
      QAction    *actionClearRecentMenu;
      QAction    *actionSeparator;

      QAction    *actionUndo;
      QAction    *actionRedo;

      QAction    *actionFullScreen;
      QAction    *actionSetColor;

      QAction    *actionAbout;

      QComboBox  *cbEngine;
      QComboBox  *cbTool;

      void init();
      void readSettings();
      void writeSettings();
      void createActions();
      void createMenus();
      void createToolbars();
      void createDocks();
      bool maybeSave();
      void setCurrentFile(const QString &fileName);
      void updateRecentFileActions();
      QString strippedName(const QString &fullFileName);

      void loadTools();

      MainWindow *findMainWindow(const QString &fileName);
  };

} // end namespace Avogadro
#endif
