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
#include <QSplitter>
#include <QTextEdit>

#include "avogadro/glwidget.h"
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <avogadro/moleculetreeview.h>

#include "ui_mainwindow.h"
#include "plugin.h"
#include "flowlayout.h"

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
      void openFile();
      void openRecentFile();
      bool save();
      bool saveAs();
      void revert();
      void exportGraphics();

      void fullScreen();
      void setBackgroundColor();

      void clearRecentFiles();
      void about();

      void actionTriggered();

      void documentWasModified();

      void setCurrentTool(QAction *action);
      void setCurrentTool(Tool *tool);
      void setCurrentTool(int i);

      void glMousePress(QMouseEvent *event);
      void glMouseMove(QMouseEvent *event);
      void glMouseRelease(QMouseEvent *event);

    private:
      Ui::MainWindow ui;

      Tool      *m_currentTool;
      Molecule  *m_molecule;
      QList<Tool *> m_tools;

      QString    m_currentFile;
      bool       m_modified;
      QUndoStack *m_undo;
      
      QActionGroup *m_agTools;
      FlowLayout *m_flowTools;
      QStackedLayout *m_stackedToolProperties;

      GLWidget *m_glView;
      QSplitter *m_splitCentral;
      QTextEdit *m_textMessages;
      QTabWidget *m_tabBottom;

      enum { maxRecentFiles = 10 };
      QAction    *m_actionRecentFile[maxRecentFiles];

      void constructor();
      void readSettings();
      void writeSettings();
      void connectUi();
      
      bool maybeSave();
      void setCurrentFile(const QString &fileName);
      void updateRecentFileActions();
      QString strippedName(const QString &fullFileName);

      void loadPlugins();

      MainWindow *findMainWindow(const QString &fileName);
      
  };

} // end namespace Avogadro
#endif
