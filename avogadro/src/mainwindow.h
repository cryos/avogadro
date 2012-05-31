/**********************************************************************
  MainWindow.h - main window, menus, main actions

  Copyright (C) 2006-2007 by Geoffrey R. Hutchison
  Copyright (C) 2006-2008 by Donald E. Curtis
  Copyright (C) 2007-2009 by Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Eigen/Geometry>
#include "ui_mainwindow.h"
#include "flattabwidget.h"

#include <avogadro/primitivelist.h>
#include <avogadro/extension.h>
#include <avogadro/glwidget.h>

#include <QMainWindow>
#include <QFileDialog>
#include <QTabWidget>

class QUndoStack;
class QUndoCommand;
class QStackedLayout;
class QStandardItem;

#ifdef QTTESTING
class pqTestUtility;
#endif

namespace OpenBabel{
  class OBFormat;
  class OBMol;
}

namespace Avogadro {
  class Tool;
  class Primitive;
  class GLWidget;
  class Molecule;
  class Engine;
  class Extension;
#ifdef ENABLE_UPDATE_CHECKER
  class UpdateCheck;
#endif

  class MainWindowPrivate;
  class MainWindow : public QMainWindow
  {
    Q_OBJECT

    public:
      MainWindow();
      MainWindow(const QString &fileName);
      ~MainWindow();

      int painterQuality() const;
      int fogLevel() const;
      bool renderAxes() const;
      bool renderDebug() const;
      bool quickRender() const;

      /**
       * Return projection type in GLWiget
       * @return projection type
      **/
      GLWidget::ProjectionType projection() const;

      /**
       * @param fileName fileName to load (defaults to loading a blank file)
       * @return whether the loading process was successful
       */
      bool loadFile(const QString &fileName = QString(),
            OpenBabel::OBFormat *format = NULL,
            const QString &options = QString());

      /**
       * @param fileName the filename to save the currently loaded file to
       */
      bool saveFile(const QString &fileName,
            OpenBabel::OBFormat *format = NULL);

      /**
       * @param noConfig Ignore any configuration options on open.
       */
      void setIgnoreConfig(bool noConfig);

      /**
       * @return true if the config file will be ignored, false otherwise.
       */
      bool ignoreConfig() const;

    protected:
      void closeEvent(QCloseEvent *event);
      bool event(QEvent *event);

      // Handle drag and drop -- accept files dragged on the window
      void dragEnterEvent(QDragEnterEvent *event);
      void dropEvent(QDropEvent *event);

    public Q_SLOTS:
      void newFile();

      /**
       * @param fileName file to load. if no filename is given, present a dialog
       */
      void openFile(QString fileName = QString());
      void openRecentFile();
      bool save();
      bool saveAs();
      void revert();
      void importFile();
      void exportGraphics();

      void closeFile();

      void cut();
      void copy();
      void copyAsSMILES();
      void copyAsInChI();
      void paste();
      void clear();
      void selectAll();
      void selectNone();

      void show();
      void showAndActivate();

      void newView();
      void duplicateView();
      void detachView();
      void closeView();
      void closeView(int index);
      void centerView();
      void alignViewToAxes();
      void setView(int index);
      void fullScreen();
      void resetDisplayTypes();
      void showSettingsDialog();
      void setBackgroundColor();
      void setPainterQuality(int quality);
      void setFogLevel(int level);

      /**
       * Slot to switch glWidget to the perspective projection mode
       */
      void setPerspective();
      /**
       * Slot to switch glWidget to the orthographic projection mode
       */
      void setOrthographic();

      void setRenderAxes(bool render);
      void setRenderDebug(bool render);
      void setQuickRender(bool quick);
      void showAllMolecules(bool show);

      void undoStackClean(bool clean);

      void updateWindowMenu();
      void bringAllToFront();
      void zoom();

      void clearRecentFiles();
      void about();
      void openTutorialURL() const;
      void openReleaseNotesURL() const;
      void openWebsiteURL() const;
      void openBugURL() const;
      void openFAQURL() const;

      void actionTriggered(); // A QAction from an Extension (e.g., a direct menu command)
      void performCommand(QUndoCommand *); // Delayed action from an Extension
      void documentWasModified();

      void setTool(Tool *tool);

      // Mac helper
      void macQuit();

      //void projectTreeItemClicked(QTreeWidgetItem *item, int column);
      void setupProjectTree();
      void projectItemActivated(const QModelIndex& index);

      void reloadTools();
      void reloadPlugins();

      /**
       * Add an extension's actions to the menu
       */
      void addActionsToMenu(Extension *extension);

      /**
       * @param molecule set the current molecule and take ownership
       * @param options Enum found in Extension with options for new Molecule
       */
      void setMolecule(Molecule *molecule, int options = Extension::DeleteOld);

      /**
       * Slot to check we have the correct active GLWidget.
       */
      void glWidgetActivated(GLWidget *glWidget);

    Q_SIGNALS:
      void moleculeChanged(Molecule *);
      /**
       * Currently used by the detached views to detect when the parent window
       * is closed.
       */
      void windowClosed();

#ifdef QTTESTING
    protected Q_SLOTS:
      void record();
      void play();
      void popup();
#endif

    private:
      friend class MainWindowPrivate;
      MainWindowPrivate * const d;
      static const int m_configFileVersion;

      bool m_ignoreConfig;

#ifdef QTTESTING
      pqTestUtility *TestUtility;
#endif

#ifdef ENABLE_UPDATE_CHECKER
      UpdateCheck *m_updateCheck;
#endif

      Ui::MainWindow ui;

      enum { maxRecentFiles = 10 };

      void constructor();
      void readSettings();
      void writeSettings();
      void connectUi();

      bool maybeSave();
      void setFileName(const QString &fileName);
      void updateRecentFileActions();

      bool isDefaultFileName(const QString fileName);
      QString defaultFileName();

      /***
       * Function for creating new GLWidgets
       * \return a pointer to a new GLWidget which has been added to the
       * internal list of all glwidgets, had it's UndoStack and ToolGroup
       * setup correctly, connects the correct signals, sets it to the
       * current widget if there is no other currently, and if there is a
       * pre-existing GLWidget it shares the GL context.
       */
      GLWidget *newGLWidget();

      //! Find an existing main window by filename
      //! \return a pointer to the appropriate window, or NULL if none exists
      MainWindow* findMainWindow(const QString &fileName);

      /**
       * @return the current molecule for this window
       */
      Molecule* molecule() const;

      /**
       * load all available extensions
       */
      void loadExtensions();

      //! Helper function for cut or copy -- prepare a clipboard
      QMimeData* prepareClipboardData(PrimitiveList selectedItems, const char* format = NULL);

      //! Helper function to paste data from mime data
      bool pasteMimeData(const QMimeData *mimeData);

      //! Helper function to check for 3D coordinates from files
      void check3DCoords(OpenBabel::OBMol *molecule);

      //! Helper function to guess text format for pasting
      bool parseText(OpenBabel::OBMol *mol, const QString coord);

      //! Helper function for Mac -- hide main window and disable menus
      void hideMainWindowMac();
      //! Helper function for Mac -- show main window and re-enable menus
      void showMainWindowMac();

      static QStringList pluginSearchDirs();

    private Q_SLOTS:

      void firstMolReady();
      void finishLoadFile();

      // select a molecule out of a multi-molecule file
      void selectMolecule(int index, int column);

      void engineSettingsClicked(Engine *engine);
      void addEngineClicked();
      void duplicateEngineClicked();
      void removeEngineClicked();
      void engineClicked(Engine *engine);

      void toggleToolSettingsDock();
      void toggleEngineSettingsDock();

      void centerStep();
  };

} // end namespace Avogadro
#endif
