/**********************************************************************
  MainWindow - main window, menus, main actions

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

#include "mainwindow.h"

#include "config.h" // krazy:exclude=includes

#include "aboutdialog.h"
#include "addenginedialog.h"
#include "editcommands.h"
#include "importdialog.h"
#include "settingsdialog.h"
#include "pluginsettings.h"
#include "savedialog.h"

#include "engineitemmodel.h"
#include "enginelistview.h"
#include "engineprimitiveswidget.h"
#include "primitiveitemmodel.h"
#include "enginecolorswidget.h"

#include <avogadro/openbabelwrapper.h>
#include <avogadro/moleculefile.h>

#ifdef ENABLE_UPDATE_CHECKER
  #include "updatecheck.h"
#endif

//#ifdef Q_WS_MAC
//#include "macchempasteboard.h"
//#endif

// Include the GL2PS header
#include "../gl2ps/gl2ps.h"

#include <avogadro/pluginmanager.h>

// Does not work for me with out of source builds at least - ui_projecttreeeditor.h
// can't be found and avogadro should be able to build without libavogadro
#include "projecttreeeditor.h"
#include "projecttreemodel.h"

#include <avogadro/camera.h>
#include <avogadro/extension.h>
#include <avogadro/engine.h>

#include <avogadro/glgraphicsview.h>

#include <avogadro/primitive.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include <avogadro/toolgroup.h>
#include <avogadro/color.h>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/builder.h>
#include <openbabel/forcefield.h>

#ifdef ENABLE_PYTHON
#include <avogadro/pythonerror.h>
#endif

#include <fstream>
#include <algorithm>

#include <QClipboard>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPluginLoader>
#include <QPushButton>
#include <QSettings>
#include <QStandardItem>
#include <QStackedLayout>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QUndoStack>
#include <QDesktopWidget>
#include <QInputDialog>
#include <QUrl>
#include <QDesktopServices>
#include <QTime>
#include <QGLFramebufferObject>
#include <QStatusBar>
#include <QListWidget>
#include <QProgressDialog>

#include <QDebug>

#include <Eigen/Geometry>
#include <Eigen/Array>
#define USEQUAT
// This is a "hidden" exported Qt function on the Mac for Qt-4.x.
#ifdef Q_WS_MAC
 void qt_mac_set_menubar_icons(bool enable);
#endif

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro
{

  class MainWindowPrivate
  {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    MainWindowPrivate() : molecule( 0 ),
      undoStack( 0 ), toolsLayout( 0 ),
      toolSettingsStacked(0), toolSettingsWidget(0), toolSettingsDock(0),
      currentSelectedEngine(0),
      messagesText( 0 ),
      glWidget(0),
      centralLayout(0), centralTab(0), bottomFlat(0),
      toolGroup( 0 ),
      settingsDialog( 0 ), importFile(0),
      initialized( false ),
      centerTimer(0),
      centerTime(0),
      moleculeFile(0), currentIndex(0),
      progressDialog(0)
    {}

    Molecule  *molecule;

    QString    fileName;
    QString    fileDialogPath;
    QUndoStack *undoStack;

    QVBoxLayout *toolsLayout;

    // we must manage this if we want it to be dynamic
    QStackedLayout *toolSettingsStacked;
    QWidget *toolSettingsWidget;
    QDockWidget *toolSettingsDock;

    QStackedLayout *enginesStacked;
    Engine         *currentSelectedEngine; // for settings widget title, etc.

    QTextEdit *messagesText;

    QList<GLWidget *> glWidgets;
    GLWidget *glWidget;

    QVBoxLayout *centralLayout;
    QTabWidget *centralTab;
    FlatTabWidget *bottomFlat;

    ToolGroup *toolGroup;
    QAction    *actionRecentFile[MainWindow::maxRecentFiles];

    SettingsDialog *settingsDialog;
    ImportDialog *importFile;

    // used for hideMainWindowMac() / showMainWindowMac()
    // save enable/disable status of every menu item
    QVector< QVector <bool> > menuItemStatus;
    bool initialized;

    bool fileToolbar;
    bool statusBar;

    Quaterniond startOrientation, endOrientation;
    Vector3d deltaTrans, startTrans;
    double rotationAcceleration;
    long rotationStart;
    int rotationTime;

    QTimer *centerTimer;
    int centerTime;

    PluginManager pluginManager;

    // Track all the molecules in a file
    MoleculeFile *moleculeFile;
    unsigned int currentIndex;
    QProgressDialog *progressDialog;
    QListWidget  allMoleculesList;

    QMap<Engine*, QWidget*> engineSettingsWindows;
  };

  const int MainWindow::m_configFileVersion = 2;

  unsigned int getMainWindowCount()
  {
    unsigned int mainWindowCount = 0;
    foreach( QWidget *widget, qApp->topLevelWidgets() ) {
      if ( qobject_cast<MainWindow *>( widget ) )
        mainWindowCount++;
    }
    return mainWindowCount;
  }

  MainWindow::MainWindow() : QMainWindow( 0 ), d( new MainWindowPrivate )
  {
    constructor();
  }

  MainWindow::MainWindow( const QString &fileName) : QMainWindow( 0 ),
  d( new MainWindowPrivate )
  {
    constructor();
    d->fileName = fileName;
  }

  MainWindow::~MainWindow()
  {
    delete(d);
  }

  void MainWindow::constructor()
  {
    ui.setupUi( this );

    QSettings settings;

    d->centralLayout = new QVBoxLayout(ui.centralWidget);

    // settings relies on the centralTab widget
    d->centralTab = new QTabWidget(ui.centralWidget);
    d->centralTab->setObjectName("centralTab");
    d->centralTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->centralLayout->addWidget(d->centralTab);

    setAttribute( Qt::WA_DeleteOnClose );
    setAcceptDrops(true);

    // add our bottom flat tabs
    d->bottomFlat = new FlatTabWidget(this);
    d->centralLayout->addWidget(d->bottomFlat);

    d->undoStack = new QUndoStack( this );

    d->toolGroup = new ToolGroup( this );
    connect(&(d->pluginManager), SIGNAL(reloadPlugins()),
            this, SLOT(reloadPlugins()));

    ui.enginesWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    d->enginesStacked = new QStackedLayout( ui.enginesWidget );

    // create messages widget
    QWidget *messagesWidget = new QWidget(this);
    QVBoxLayout *messagesVBox = new QVBoxLayout( messagesWidget );
    d->messagesText = new QTextEdit(this);
    d->messagesText->setReadOnly( true );

#ifdef ENABLE_PYTHON
    connect(PythonError::instance(), SIGNAL(message(const QString&)),
            d->messagesText, SLOT(append(const QString&)));
    d->messagesText->append( PythonError::instance()->string() );
    PythonError::instance()->setListening(true); // switch to 'listening mode'
#endif

    messagesVBox->setMargin( 3 );
    messagesVBox->addWidget( d->messagesText );
    d->bottomFlat->addTab( messagesWidget, tr( "Messages" ) );

    // put in the recent files
    for ( int i = 0; i < maxRecentFiles; ++i ) {
      d->actionRecentFile[i] = new QAction( this );
      d->actionRecentFile[i]->setVisible( false );
      ui.menuOpenRecent->addAction( d->actionRecentFile[i] );
      connect( d->actionRecentFile[i], SIGNAL( triggered() ),
              this, SLOT( openRecentFile() ) );
    }
    ui.menuOpenRecent->addSeparator();
    ui.menuOpenRecent->addAction( ui.actionClearRecent );

    QAction *undoAction = d->undoStack->createUndoAction( this );
    undoAction->setIcon( QIcon( QString::fromUtf8( ":/icons/undo.png" ) ) );
    undoAction->setShortcuts( QKeySequence::Undo );
    QAction *redoAction = d->undoStack->createRedoAction( this );
    redoAction->setIcon( QIcon( QString::fromUtf8( ":/icons/redo.png" ) ) );
    redoAction->setShortcuts( QKeySequence::Redo );
    if ( ui.menuEdit->actions().count() ) {
      QAction *firstAction = ui.menuEdit->actions().at( 0 );
      ui.menuEdit->insertAction( firstAction, redoAction );
      ui.menuEdit->insertAction( redoAction, undoAction );
    } else {
      ui.menuEdit->addAction( undoAction );
      ui.menuEdit->addAction( redoAction );
    }

    ui.menuToolbars->addAction( ui.projectDock->toggleViewAction() );
    ui.menuToolbars->addAction( ui.enginesDock->toggleViewAction() );
    ui.menuToolbars->addAction( ui.fileToolBar->toggleViewAction() );
    ui.menuToolbars->addAction( ui.toolBar->toggleViewAction() );

    // Disable the "Revert" and "Save" actions -- we haven't modified anything
    // This will be enabled when the document is modified
    ui.actionRevert->setEnabled(false);
    ui.actionSave->setEnabled(false);

#ifdef Q_WS_MAC
    // Find the Avogadro global preferences action
    // and make sure it ends up in the Mac Application menu
    ui.configureAvogadroAction->setMenuRole(QAction::PreferencesRole);

    // Turn off the file toolbar (not really Mac-native)
    // Fixes PR#1965004
    ui.menuToolbars->removeAction( ui.fileToolBar->toggleViewAction() );
    ui.fileToolBar->hide();

    // Change the "Settings" menu to be Window
    ui.menuSettings->setTitle(tr("Window"));
    QAction *firstAction = ui.menuSettings->actions().first();
    QAction *minimizeAction = new QAction(this);
    minimizeAction->setText(tr("&Minimize"));
    minimizeAction->setShortcut(QKeySequence(tr("Ctrl+M")));
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(showMinimized()));
    ui.menuSettings->insertAction(firstAction, minimizeAction);

    QAction *zoomAction = new QAction(this);
    zoomAction->setText(tr("&Zoom"));
    connect(zoomAction, SIGNAL(triggered()), this, SLOT(zoom()));
    ui.menuSettings->insertAction(firstAction, zoomAction);
    ui.menuSettings->insertSeparator(firstAction);

    ui.menuSettings->addSeparator();
    QAction *raiseAction = new QAction(this);
    raiseAction->setText(tr("Bring All to Front"));
    connect(raiseAction, SIGNAL(triggered()), this, SLOT(bringAllToFront()));
    ui.menuSettings->addAction(raiseAction);
    ui.menuSettings->addSeparator();

    updateWindowMenu();

    // Remove all menu icons (violates Apple interface guidelines)
    // This is a not-quite-hidden Qt call on the Mac
    //    http://doc.trolltech.com/exportedfunctions.html
    qt_mac_set_menubar_icons(false);
#endif

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    connectUi();

    ui.projectDock->close();
    ui.enginesDock->close();

#ifdef ENABLE_UPDATE_CHECKER
    m_updateCheck = new UpdateCheck(this);
#endif

    // Disable the detach view option for now
    // FIXME
    ui.actionDetachView->setVisible(false);
  }

  bool MainWindow::event(QEvent *event)
  {
    // delayed initialization
    if(event->type() == QEvent::Polish) {
      reloadTools();
      if (d->toolSettingsDock)
        d->toolSettingsDock->hide();
      loadExtensions();

      // Check every menu for "extra" separators
      QList<QAction *> removeThese;
      foreach( QAction *menu, menuBar()->actions() ) {
        if (menu->menu()->actions().isEmpty())
          continue;

        removeThese.clear();

        QAction *firstAction = menu->menu()->actions().first();
        if (firstAction->isSeparator())
          removeThese.append( firstAction );

        QAction *lastAction = menu->menu()->actions().last();
        if (lastAction->isSeparator())
          removeThese.append( lastAction );

        int multipleSeparatorCount = 0;
        foreach( QAction *menuItem, menu->menu()->actions() ) {
          if (menuItem->isSeparator()) {
            if (multipleSeparatorCount)
              removeThese.append(menuItem);
            multipleSeparatorCount++;
          } else
            multipleSeparatorCount = 0;
        } // end foreach (menuItems)

        foreach (QAction *separator, removeThese) {
          menu->menu()->removeAction(separator);
        }
      }

      if(!molecule()) {
        loadFile();
      }
      // read settings
      readSettings();
      // if we don't have a molecule then load a blank file
      d->initialized = true;
    }
    else if(event->type() == QEvent::ActivationChange
            || event->type() == QEvent::WindowActivate) {
#ifdef Q_WS_MAC
      updateWindowMenu();
#endif
    }

    return QMainWindow::event(event);
  }

  void MainWindow::dragEnterEvent(QDragEnterEvent *event)
  {
    if (event->mimeData()->hasUrls())
      event->acceptProposedAction();
    else
      event->ignore();
  }

  void MainWindow::dropEvent(QDropEvent *event)
  {
    if (event->mimeData()->hasUrls()) {
      foreach(const QUrl& url, event->mimeData()->urls() ) {
        loadFile(url.toLocalFile());
      }
      event->acceptProposedAction();
    }
    else
      event->ignore();
  }

  void MainWindow::show()
  {
    QMainWindow::show();
  }

  bool MainWindow::renderAxes() const
  {
    return d->glWidget->renderAxes();
  }

  void MainWindow::setRenderAxes(bool render)
  {
    d->glWidget->setRenderAxes(render);
  }

  bool MainWindow::renderDebug() const
  {
    return d->glWidget->renderDebug();
  }

  void MainWindow::setRenderDebug(bool render)
  {
    d->glWidget->setRenderDebug(render);
  }

  bool MainWindow::quickRender() const
  {
    // Is the current widget using quick render?
    return d->glWidget->quickRender();
  }

  void MainWindow::setQuickRender(bool quick)
  {
    d->glWidget->setQuickRender(quick);
  }

  bool MainWindow::renderUnitCellAxes() const
  {
    // Is the current widget showing a unit cell frame?
    return d->glWidget->renderUnitCellAxes();
  }

  void MainWindow::setRenderUnitCellAxes(bool render)
  {
    ui.actionDisplayUnitCellAxes->setChecked(false);
    d->glWidget->setRenderUnitCellAxes(render);
  }

  void MainWindow::showAllMolecules(bool showWindow)
  {
    if (showWindow) {
      d->allMoleculesList.show();
      d->allMoleculesList.raise();
    } else {
      d->allMoleculesList.hide();
    }
  }

  void MainWindow::reloadPlugins()
  {
    qDebug() << "MainWindow::reloadPlugins";
    /**
     * Extensions: instances are deleted by the PluginManager after writing the
     * settings. The QActions are removed from the menus when they are deleted.
     * So we only have to load new extensions.
     */
    loadExtensions();

    /**
     * Engines: Clear all the EngineListViews and call GLWidget::reloadEngines()
     * for each GLWidget.
     */
    foreach (GLWidget *glwidget, d->glWidgets)
      glwidget->reloadEngines();


    int count = d->enginesStacked->count();
    for (int i = 0; i < count; ++i) {
      QWidget *widget = d->enginesStacked->widget(i);
      foreach(QObject *object, widget->children()) {
        if (!object->isWidgetType())
          continue;
        EngineListView *engineListView = qobject_cast<EngineListView*>(object);
        if (engineListView)
          engineListView->clear();
      }
    }

    /**
     * Tools: see reloadTools().
     */
    reloadTools();
    qDebug() << "end MainWindow::reloadPlugins";
  }

  void MainWindow::reloadTools()
  {
    if(d->toolSettingsDock) {
      delete d->toolSettingsDock;
      d->toolSettingsDock = 0;
      d->toolSettingsWidget = 0;
      d->toolSettingsStacked = 0;
    }

    ui.toolBar->clear();

    d->toolSettingsDock = new QDockWidget(this);
    d->toolSettingsDock->setObjectName(QString::fromUtf8("toolSettingsDock"));
    d->toolSettingsDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::NoDockWidgetArea|Qt::RightDockWidgetArea);
    d->toolSettingsWidget = new QWidget(d->toolSettingsDock);
    d->toolSettingsWidget->setObjectName(QString::fromUtf8("toolSettingsWidget"));
    d->toolSettingsDock->setWidget(d->toolSettingsWidget);

    addDockWidget(static_cast<Qt::DockWidgetArea>(1), d->toolSettingsDock);
    ui.menuToolbars->addAction( d->toolSettingsDock->toggleViewAction() );

    // Make sure to do this first, or we'll crash
    d->toolSettingsStacked = new QStackedLayout( d->toolSettingsWidget );
    // Add a blank widget for those tools with no settings widget
    // (slot 0)
    QWidget* blankWidget = new QWidget;
    blankWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->toolSettingsStacked->addWidget(blankWidget);

    d->toolGroup->removeAllTools();
    d->toolGroup->append(d->pluginManager.tools(this));

    const QList<Tool *> tools = d->toolGroup->tools();
    int toolCount = tools.size();

    for ( int i = 0; i < toolCount; i++ ) {
      Tool *tool = tools.at(i);
      connect(tool, SIGNAL(message(QString)), d->messagesText,
              SLOT(append(QString)));

      QAction *action = tool->activateAction();
      // Add Ctrl + tool number as an added shortcut
      int toolNumber = i+1;
      if (toolNumber == 10)
        toolNumber = 0;
      if (toolNumber <= 9) {
        // If we have 11 or more tools, we can only do this for the first 10
        QList<QKeySequence> shortcuts = action->shortcuts();
        shortcuts.append(QKeySequence(QString("Ctrl+%1").arg(toolNumber)));
        action->setShortcuts(shortcuts);
      }

      ui.toolBar->addAction( action );

      QWidget *widget = tools.at( i )->settingsWidget();
      if(widget){
        d->toolSettingsStacked->addWidget( widget );
        if ( i == 0 ) {
          d->toolSettingsStacked->setCurrentIndex( 1 );
        }
      }

    } // end for loop

    // Add buttons to toggle the tool and engine settings docks
    ui.toolBar->addSeparator();

    QPushButton* toolSettings = new QPushButton(tr("&Tool Settings..."), ui.toolBar);
    toolSettings->setCheckable(true);
    toolSettings->setChecked(d->toolSettingsDock->isVisible());
    toolSettings->setShortcut(tr("Ctrl+T", "Tool Settings shortcut"));
    connect(d->toolSettingsDock, SIGNAL(visibilityChanged(bool)), toolSettings, SLOT(setChecked(bool)));
    connect(toolSettings, SIGNAL(released()), this, SLOT(toggleToolSettingsDock()));
    ui.toolBar->addWidget(toolSettings);

    QPushButton* displaySettings = new QPushButton(tr("&Display Settings..."), ui.toolBar);
    displaySettings->setCheckable(true);
    displaySettings->setChecked(ui.enginesDock->isVisible());
    displaySettings->setShortcut(tr("Ctrl+D", "Display Settings shortcut"));
    connect(ui.enginesDock, SIGNAL(visibilityChanged(bool)), displaySettings, SLOT(setChecked(bool)));
    connect(displaySettings, SIGNAL(released()), this, SLOT(toggleEngineSettingsDock()));
    ui.toolBar->addWidget(displaySettings);

    // Now, set the active tool
    if (d->molecule)
      d->toolGroup->setActiveTool("Navigate");
  } // end reloadTools

  void MainWindow::newFile()
  {
#ifdef Q_WS_MAC
    unsigned int mainWindowCount = getMainWindowCount();

    if ( mainWindowCount == 1 && !isVisible() ) {
      showMainWindowMac();
      return;
    }
#endif

    writeSettings();
    MainWindow *other = new MainWindow;
#ifdef Q_WS_MAC
    other->move( x() + 25, y() + 25 );
#endif
    other->show();
  }

  void MainWindow::openFile( QString fileName )
  {
    // no parameter give create dialog
    if ( fileName.isEmpty() )
    {
      QSettings settings;
      QString selectedFilter = settings.value("Open Molecule Filter").toString();

      QStringList filters;
      filters << tr("Common molecule formats")
        + " (*.cml *.xyz *.pdb *.alc *.cdx *.cdxml *.ent"
          " *.gpr *.mdl *.mol *.sdf *.sd *.dmol"
          " *.inp *.gamin *.gamout *.fract *.gjf *.gzmat"
          " *.mol2 *.nwo *.out *.log *.pqr)"
        << tr("All files") + " (* *.*)"
        << tr("CML") + " (*.cml)"
        << tr("Computational Chemistry Output") + " (*.out *.log *.dat *.output)"
        << tr("Crystallographic Interchange CIF") + " (*.cif)"
        << tr("GAMESS-US Output") + " (*.gamout)"
        << tr("Gaussian 98/03 Output") + " (*.g98 *.g03)"
        << tr("Gaussian Formatted Checkpoint") + " (*.fchk)"
        << tr("HyperChem") + " (*.hin)"
        << tr("MDL Mol") + " (*.mdl *.mol *.sd *.sdf)"
        << tr("NWChem Output") + " (*.nwo)"
        << tr("PDB") + " (*.pdb *.ent)"
        << tr("Sybyl Mol2") + " (*.mol2)"
        << tr("XYZ") + " (*.xyz)";

      fileName = QFileDialog::getOpenFileName( this,
        tr( "Open File" ), d->fileDialogPath, filters.join(";;"), &selectedFilter);
      settings.setValue("Open Molecule Filter", selectedFilter);
    }

    if ( !fileName.isEmpty() ) {
      d->fileDialogPath = QFileInfo(fileName).absolutePath();

      // First check if we closed all the windows on Mac
      // if so, show the hidden window
#ifdef Q_WS_MAC
      unsigned int mainWindowCount = getMainWindowCount();

      if ( mainWindowCount == 1 && isHidden() ) {
        showMainWindowMac();
      }
#endif

      // check to see if we already have an open window
      MainWindow *existing = findMainWindow( fileName );
      if ( existing ) {
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
      }

      // if we have nothing open or modified
      if ( d->fileName.isEmpty() && !isWindowModified() ) {
        loadFile( fileName );
      } else {
        // ONLY if we have loaded settings then we can write them
        if(d->initialized) {
          writeSettings();
        }
        MainWindow *other = new MainWindow();
        if ( !other->loadFile( fileName ) ) {
          delete other;
          return;
        }
#if defined (Q_WS_MAC) || defined (Q_WS_WIN)
        other->move( x() + 25, y() + 25 );
#endif
        other->show();
      }
    }
  }

  void MainWindow::openRecentFile()
  {
    QAction *action = qobject_cast<QAction *>( sender() );
    if ( action ) {
      openFile( action->data().toString() );
    }
  }

  bool MainWindow::loadFile(const QString &fileName,
                            OBFormat *format,
                            const QString &options)
  {
    // Now also sets the window icon on Mac
    // http://labs.trolltech.com/blogs/2007/12/28/spotlight-on-little-things/
    QString shownName = fileName;
    if(fileName.isEmpty()) {
      setFileName(fileName);
      setMolecule(new Molecule(this));
      ui.actionAllMolecules->setEnabled(false); // only one molecule -- the blank slate
      return true;
    }

    statusBar()->showMessage( tr("Loading %1...", "%1 is a filename").arg(fileName), 5000 );

    QApplication::setOverrideCursor( Qt::WaitCursor );
    statusBar()->showMessage( tr("Loading %1...").arg(fileName), 5000 );

    QString formatType;
    if (format != NULL) {
      formatType = format->GetID();
    }
    // This will work in a background thread -- we want to wait until the firstMolReady() signal appears
    d->moleculeFile = OpenbabelWrapper::readFile(fileName, formatType.trimmed(), options, false);
    if (!d->moleculeFile)
      return false;

    // TODO: split into first molecule vs. whole file
    // TODO: pop up progress dialog
    connect(d->moleculeFile, SIGNAL(ready()), this, SLOT(firstMolReady()));
    connect(d->moleculeFile, SIGNAL(ready()), this, SLOT(finishLoadFile()));

    return true;
  }

  void MainWindow::check3DCoords(OBMol *obMolecule)
  {
    if (obMolecule->GetDimension() != 3) {
      int retval = QMessageBox::warning( this, tr( "Avogadro" ),
                                         tr( "This file contains does not contain 3D coordinates.\n"
                                             "Do you want Avogadro to build a rough geometry?"),
                                         QMessageBox::Yes, QMessageBox::No );

      if (retval == QMessageBox::Yes) {
        // In OB-2.2.2 and later, builder will use 2D coordinates if present
        OBBuilder builder;
        builder.Build(*obMolecule);
        obMolecule->AddHydrogens(false, true); // Add some hydrogens before running force field

        OBForceField* pFF =  OBForceField::FindForceField("UFF");
        if (pFF && pFF->Setup(*obMolecule)) {
          pFF->ConjugateGradients(250, 1.0e-4);
          pFF->UpdateCoordinates(*obMolecule);
        }
      }
      else {
        QMessageBox::warning( this, tr( "Avogadro" ),
                              tr( "This file does not contain 3D coordinates.\n"
                                  "You may not be able to edit or view properly." ));
      }
    }
  }

  void MainWindow::selectMolecule(int index)
  {
    if (!d->moleculeFile)
      return; // nothing to do

    OBMol *obMolecule =  d->moleculeFile->OBMol(index);
    if (!obMolecule)
      return; // bad index
    check3DCoords(obMolecule);

    Molecule *mol = new Molecule;
    mol->setOBMol(obMolecule);
    setMolecule(mol);
  }

  void MainWindow::finishLoadFile()
  {
    if (!d->moleculeFile)
      return;

    if (d->moleculeFile->numMolecules() > 1)
      ui.actionAllMolecules->setEnabled(true); // only one molecule -- the blank slate

    d->allMoleculesList.clear();
    d->allMoleculesList.addItems(d->moleculeFile->titles());

    connect(&d->allMoleculesList, SIGNAL(currentRowChanged(int)), this, SLOT(selectMolecule(int)));
  }

  void MainWindow::firstMolReady()
  {
    if (d->moleculeFile == NULL)
      return;

    ui.actionAllMolecules->setEnabled(false); // only one molecule right now

    QString errors = d->moleculeFile->errors();
    OBMol *obMolecule = d->moleculeFile->OBMol();
    if (errors.isEmpty() && obMolecule != NULL) { // successful read

      qDebug() << " read " << d->moleculeFile->numMolecules() << " molecules.";
      foreach(const QString &title, d->moleculeFile->titles())
        qDebug() << title;

      // e.g. SMILES or MDL molfile, etc.
      check3DCoords(obMolecule);

      Molecule *mol = new Molecule;
      mol->setOBMol(obMolecule);
      setMolecule(mol);
      // Now unroll any settings we saved in the file
      // This is disabled for version 1.0-release
      /*
      if (obMolecule->HasData(OBGenericDataType::PairData)) {
        QSettings settings;
        // We've saved the settings with key Avogadro:blah as an OBPairData.
        std::vector<OBGenericData *> pairDataVector = obMolecule->GetAllData(OBGenericDataType::PairData);
        OBPairData *savedSetting;
        OBDataIterator i;
        QString attribute;

        for (i = pairDataVector.begin(); i != pairDataVector.end(); ++i) {
          savedSetting = dynamic_cast<OBPairData *>(*i);
          // Check to see if this is an Avogadro setting
          attribute = savedSetting->GetAttribute().c_str();
          if (attribute.startsWith(QLatin1String("Avogadro:"))) {
            attribute.remove(QLatin1String("Avogadro:"));
            settings.setValue(attribute, savedSetting->GetValue().c_str());
            // TODO: we should probably delete the entry now, but I'm going to play it safe first
          }
        }
      } // end reading OBPairData
      */

      QApplication::restoreOverrideCursor();

      // Check if this is a PDB file -- by default we do not show the unit cell
      QFileInfo info(d->moleculeFile->fileName());
      if (d->moleculeFile->fileType().contains("PDB", Qt::CaseInsensitive)
          || info.completeSuffix().contains("PDB", Qt::CaseInsensitive)) {
        setRenderUnitCellAxes(false);
      }

      QString status;
      QTextStream( &status ) << tr("Atoms: ") << d->molecule->numAtoms() <<
        tr(" Bonds: ") << d->molecule->numBonds();
      statusBar()->showMessage( status, 5000 );
    }
    else { // errors
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this, tr("Avogadro"),
                           tr("Reading molecular file failed, file %1.").arg(d->moleculeFile->fileName()));
      return;
    }

    setFileName( d->moleculeFile->fileName() );
    setWindowFilePath(d->moleculeFile->fileName()); // for MacOS X
#ifdef Q_WS_MAC
    updateWindowMenu();
#endif
    statusBar()->showMessage( tr("File Loaded..."), 5000 );
    d->toolGroup->setActiveTool("Navigate");
  }

  bool MainWindow::maybeSave()
  {
    if ( isWindowModified() ) {
      // We're using the property interface to QMessageBox, rather than
      // the static functions. This is more work, but gives us some nice
      // fine-grain control. This helps both on Windows and Mac
      // look more "native."
      QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                         tr( "Avogadro" ),
                         tr( "Do you want to save the changes you made in the document?" ),
                         QMessageBox::Save | QMessageBox::Discard
                         | QMessageBox::Cancel,
                         this);

      // On Mac, this will make a sheet relative to the window
      // Unfortunately, it also closes the window when the box disappears!
      // msgBox->setWindowModality(Qt::WindowModal);
      // second line of text
      msgBox->setInformativeText(tr("Your changes will be lost if you don't save them." ));
      msgBox->setDefaultButton(QMessageBox::Save);

      // OK, now add shortcuts for save and discard
      msgBox->button(QMessageBox::Save)->setShortcut(QKeySequence(tr("Ctrl+S", "Save")));
      msgBox->button(QMessageBox::Discard)->setShortcut(QKeySequence(tr("Ctrl+D", "Discard")));
      msgBox->setButtonText(QMessageBox::Save,
                            d->fileName.isEmpty() ? tr("Save...") : tr("Save"));

      int ret = msgBox->exec();

      if ( ret == QMessageBox::Save ) {
        delete msgBox;
        return save();
      }
      else if ( ret == QMessageBox::Cancel ) {
        delete msgBox;
        return false;
      }
      delete msgBox;
    }
    return true;
  }

  // Close the current file -- leave an empty window
  // Not used on Mac: the window is closed via closeEvent() instead
  void MainWindow::closeFile()
  {
    if ( maybeSave() ) {
      d->undoStack->clear();
      loadFile();
    }
  }

  void MainWindow::closeEvent( QCloseEvent *event )
  {
    raise();

#ifdef Q_WS_MAC
    //    qDebug() << " close event ";
    unsigned int mainWindowCount = getMainWindowCount();

    if ( mainWindowCount == 1 && isVisible() ) {
      if ( maybeSave() ) {
        writeSettings();

        // Clear the undo stack first (or we'll have an enabled Undo command)
        d->undoStack->clear();
        hideMainWindowMac();
      }
      event->ignore();
      return;
    }
#endif

    if ( maybeSave() ) {
      writeSettings();
      event->accept();
    } else {
      event->ignore();
    }
  }

  bool MainWindow::save()
  {
    if ( d->fileName.isEmpty() ) {
      return saveAs();
    } else {
      return saveFile( d->fileName );
    }
  }

  bool MainWindow::saveAs()
  {
    QSettings settings;
    QString selectedFilter = settings.value("Save Molecule Filter", tr("CML") + " (*.cml)").toString();

    QStringList filters;
    filters << tr("All files") + " (* *.*)"
// Omit these on Mac, since it doesn't match "native" save dialogs
#ifndef Q_WS_MAC
            << tr("Common molecule formats")
                      + " (*.cml *.xyz *.ent *.pdb *.alc *.chm *.cdx *.cdxml *.c3d1 *.c3d2"
                        " *.gpr *.mdl *.mol *.sdf *.sd *.crk3d *.cht *.dmol *.bgf"
                        " *.gam *.inp *.gamin *.gamout *.tmol *.fract *.gau *.gzmat"
                        " *.mpd *.mol2 *.nwo)"
#endif
            << tr("CML") + " (*.cml)"
            << tr("GAMESS Input") + " (*.gamin)"
            << tr("Gaussian Cartesian Input") + " (*.gau)"
            << tr("Gaussian Z-matrix Input") + " (*.gzmat)"
            << tr("MDL SDfile") + "(*.mol)"
            << tr("PDB") + " (*.pdb)"
            << tr("NWChem Input") + " (*.nw)"
            << tr("Sybyl Mol2") + " (*.mol2)"
            << tr("XYZ") + " (*.xyz)";

    QString fileName = SaveDialog::run(this,
                                       tr("Save Molecule As"),
                                       d->fileDialogPath,
                                       d->fileName,
                                       filters,
                                       "cml",
                                       selectedFilter);
    settings.setValue("Save Molecule Filter", selectedFilter);

    if(fileName.isEmpty())
    {
      return false;
    }

    // we must save the file before we can set the fileName
    bool result = saveFile( fileName );

    setFileName( fileName );

    return result;
  }

  bool MainWindow::saveFile( const QString &fileName, OBFormat *format )
  {
    QString formatType;
    if (format) {
      formatType = format->GetID();
    }

    if (!d->moleculeFile) {
      // just save this one molecule
      OpenbabelWrapper::writeMolecule(d->molecule, fileName, formatType.trimmed());
    } else {
      if (d->moleculeFile->isConformerFile())
        OpenbabelWrapper::writeConformers(d->molecule, fileName, formatType.trimmed());
      else
        // use MoleculeFile to save just the current slice of the file
        d->moleculeFile->replaceMolecule(d->currentIndex, d->molecule, fileName);
    }


    /*
    QFile file(fileName);
    bool replaceExistingFile = file.exists();

    // We'll save to a new file and then rename it to the requested file name
    // This way, if an error occurs, we won't destroy the old file
    QString newFileName(fileName);
    if (replaceExistingFile)
      newFileName += ".new";
    QFile newFile( newFileName );
    if ( !newFile.open( QFile::WriteOnly | QFile::Text ) ) {
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot write to the file %1:\n%2." )
          .arg( fileName )
          .arg( newFile.errorString() ) );
      return false;
    }
    newFile.close();

    // Pass of an ofstream to Open Babel
    ofstream ofs;
    ofs.open( QFile::encodeName(newFileName) );
    if ( !ofs ) { // shouldn't happen, already checked file above
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot write to the file %1." )
          .arg( fileName ) );
      return false;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );
    statusBar()->showMessage( tr( "Saving file." ), 2000 );

    OBMol obmol = d->molecule->OBMol();

    // We're going to wrap up the QSettings and save them to the CML file
    if (QString(outFormat->GetID()).compare("cml", Qt::CaseInsensitive) == 0) {
      // First off, let's set some CML options
      conv.AddOption("p"); // add properties -- including OBPairData
      conv.AddOption("m"); // Dublin Core metadata
      conv.AddOption("a"); // array format for atoms & bonds

      QSettings settings;
      OBPairData *savedSetting;
      QString attribute, value;

      // Walk through all our settings (is there a more efficient way to do this?)
      foreach(const QString &key, settings.allKeys()) {
        // Ignore this key -- there are definitely some on Mac with Apple... or com/
        // There may be others to ignore on Linux and Windows, but I haven't tested those yet.
        if (key.startsWith(QLatin1String("Apple"))
            || key.startsWith(QLatin1String("com/"))
            || key.startsWith(QLatin1String("NS")))
          continue;

        if (key.startsWith(QLatin1String("enginesDock"))) {
          continue; // TODO: this seems to kill the CML
        }

        // We're going to save all our settings as Avogadro:blah
        savedSetting = new OBPairData;
        attribute = "Avogadro:" + key;
        // Convert from QString to char*
        savedSetting->SetAttribute(attribute.toAscii().constData());

        value = settings.value(key).toString();
        // Convert from QString to char*
        savedSetting->SetValue(value.toAscii().constData());
        savedSetting->SetOrigin(userInput);
        obmol.SetData(savedSetting);
      }
    } // end saving settings for CML files

    if ( conv.Write( &obmol, &ofs ) ) {
      ofs.close();
      if (replaceExistingFile) {
        bool success;
        success = file.rename(fileName + ".old");
        if (success) {
          file.setFileName(fileName + ".old");
          success = newFile.rename(fileName);
        }
        else {
          QApplication::restoreOverrideCursor();
          QMessageBox::warning(this, tr("Avogadro"),
              tr("Saving molecular file failed - could not rename original file"));
          return false;
        }
        if (success) // renaming worked
          success = file.remove(); // remove the old file: WARNING -- would much prefer to just rename, but Qt won't let you
        else {
          QApplication::restoreOverrideCursor();
          QMessageBox::warning(this, tr("Avogadro"),
              tr("Saving molecular file failed - could not rename new file"));
          return false;
        }

        if (success) {
          statusBar()->showMessage( tr("Save succeeded."), 5000 );
          setWindowModified( false );
        }
        else {
          QApplication::restoreOverrideCursor();
          QMessageBox::warning(this, tr("Avogadro"),
              tr("Saving molecular file failed - could not remove old file"));
          return false;
        }
      }
      else
        statusBar()->showMessage( tr("Save succeeded."), 5000 );
    }
    else {
      statusBar()->showMessage( tr("Saving molecular file failed."), 5000 );
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this, tr("Avogadro"),
          tr("Saving molecular file failed - could not rename original file"));
      newFile.remove(); // remove the temporary file -- we'll leave the old file in place
    }
    */

    QApplication::restoreOverrideCursor();

    return true;
  }


  void MainWindow::undoStackClean( bool clean )
  {
    ui.actionRevert->setEnabled(!clean);
    ui.actionSave->setEnabled(!clean);

    setWindowModified( !clean );
  }

  void MainWindow::exportGraphics()
  {
    QSettings settings;
    QString selectedFilter = settings.value("Export Graphics Filter", tr("PNG") + " (*.png)").toString();

    QStringList filters;
// Omit "common image formats" on Mac
#ifdef Q_WS_MAC
    filters
#else
    filters << tr("Common image formats")
              + " (*.png *.jpg *.jpeg)"
#endif
            << tr("All files") + " (* *.*)"
            << tr("BMP") + " (*.bmp)"
            << tr("PNG") + " (*.png)"
            << tr("JPEG") + " (*.jpg *.jpeg)";

    // Use QFileInfo to get the parts of the path we want
    QFileInfo info(d->molecule->fileName());

    QString fileName = SaveDialog::run(this,
                                       tr("Export Bitmap Graphics"),
                                       info.absolutePath(),
                                       info.baseName(),
                                       filters,
                                       "png",
                                       selectedFilter);

    settings.setValue("Export Graphics Filter", selectedFilter);

    if(fileName.isEmpty())
      return;

    qDebug() << "Exported filename:" << fileName;

    // render it (with alpha channel)
    QImage exportImage;
    d->glWidget->raise();
    d->glWidget->repaint();
    if (QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
      exportImage = d->glWidget->grabFrameBuffer( true );
    } else {
      QPixmap pixmap = QPixmap::grabWindow( d->glWidget->winId() );
      exportImage = pixmap.toImage();
    }

    // now we embed molecular information into the file, if possible
    OBConversion conv;
    // MDL format is used for main copy -- atoms, bonds, chirality
    // supports either 2D or 3D, generic data
    OBFormat *mdlFormat = conv.FindFormat( "mdl" );
    QByteArray copyData;
    OBMol obmol = d->molecule->OBMol();
    string output;
    if ( mdlFormat && conv.SetOutFormat( mdlFormat ) ) {
      output = conv.WriteString(&obmol);
      copyData = output.c_str();
      // we embed the molfile into the image
      // e.g. http://baoilleach.blogspot.com/2007/08/access-embedded-molecular-information.html
      exportImage.setText("molfile", copyData);
    }

    // save a canonical SMILES too
    OBFormat *canFormat = conv.FindFormat( "can" );
    if ( canFormat && conv.SetOutFormat( canFormat ) ) {
      output = conv.WriteString(&obmol);
      if (output.size()) { // might be empty (e.g., large protein)
        copyData = output.c_str();
        exportImage.setText("SMILES", copyData);
      }
    }
    if ( !exportImage.save( fileName ) ) {
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot save file %1." ).arg( fileName ) );
      return;
    }
  }

  void MainWindow::exportGL2PS()
  {
    QSettings settings;
    QString selectedFilter = settings.value("Export GL2PS Filter", tr("PDF")
                                            + " (*.pdf)").toString();
    QStringList filters;
// Omit "common image formats" on Mac
#ifdef Q_WS_MAC
    filters
#else
    filters << tr("Common vector image formats")
              + " (*.pdf *.svg *.eps)"
#endif
            << tr("All files") + " (* *.*)"
            << tr("PDF") + " (*.pdf)"
            << tr("SVG") + " (*.svg)"
            << tr("EPS") + " (*.eps)";

    // Use QFileInfo to get the parts of the path we want
    QFileInfo info(d->molecule->fileName());

    QString fileName = SaveDialog::run(this,
                                       tr("Export Bitmap Graphics"),
                                       info.absolutePath(),
                                       info.baseName(),
                                       filters,
                                       "pdf",
                                       selectedFilter);

    settings.setValue("Export GL2PS Filter", selectedFilter);

    if(fileName.isEmpty())
      return;

    qDebug() << "Exported filename:" << fileName;
    info.setFile(fileName);

    // Just using the example right now, this is a C library but may be the
    // file calls need cleaning up a little.
    FILE *fp;
    int state = GL2PS_OVERFLOW, buffsize = 8*1024*1024, fileType = GL2PS_PDF;

    // Enumerate through the supported file types
    if (info.suffix() == "pdf")
      fileType = GL2PS_PDF;
    else if (info.suffix() == "svg")
      fileType = GL2PS_SVG;
    else if (info.suffix() == "eps")
      fileType = GL2PS_EPS;
    else
      return;

    fp = fopen(QFile::encodeName(fileName), "wb");
    qDebug() << "Writing out a vector graphics file...";
    while(state == GL2PS_OVERFLOW) {
      buffsize += 1024*1024;
      gl2psBeginPage("test", "gl2psTestSimple", NULL, fileType, GL2PS_BSP_SORT,
                     GL2PS_DRAW_BACKGROUND
                     | GL2PS_USE_CURRENT_VIEWPORT | GL2PS_OCCLUSION_CULL
                     | GL2PS_BEST_ROOT,
                     GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp,
                     info.baseName().toStdString().c_str());
      d->glWidget->renderNow();
      state = gl2psEndPage();
    }
    fclose(fp);
    qDebug() << "Done...";
  }

  void MainWindow::revert()
  {
    if ( !d->fileName.isEmpty() ) {
      loadFile( d->fileName );
    }
  }

  void MainWindow::documentWasModified()
  {
    // Now that the document was modified, enable save/revert
    ui.actionRevert->setEnabled(true);
    ui.actionSave->setEnabled(true);
    setWindowModified( true );
  }

  // For the Mac, we need to do a little work when quitting
  // Basically, we try to close every window. If successful,
  //  then we quit
  void MainWindow::macQuit()
  {
    QCloseEvent ev;
    QApplication::sendEvent(qApp, &ev);
    if(ev.isAccepted())
      qApp->quit();
  }

  void MainWindow::clearRecentFiles()
  {
    QSettings settings; // already set up properly via main.cpp
    QStringList files;
    settings.setValue( "recentFileList", files );

    updateRecentFileActions();
  }

  void MainWindow::zoom()
  {
    setWindowState(windowState() ^ Qt::WindowMaximized);
  }

  void MainWindow::bringAllToFront()
  {
    foreach(QWidget *widget, qApp->topLevelWidgets()) {
      MainWindow *window = qobject_cast<MainWindow *>( widget );
      if (window)
        raise();
    }
  }

  bool windowComparison(const MainWindow *first, const MainWindow *second)
  {
    return first->windowTitle() > second->windowTitle();
  }

  void MainWindow::updateWindowMenu()
  {
    // first remove actions at end of Window menu
    bool removeItem = false;
    QList<QAction *> removeThese;
    foreach (QAction *menuItem, ui.menuSettings->actions()) {
      if (menuItem->text() == tr("Bring All to Front")) {
        removeItem = true;
        continue;
      }
      if (removeItem)
        removeThese.append(menuItem);
    }

    foreach (QAction *action, removeThese) {
      ui.menuSettings->removeAction(action);
    }

    QList<MainWindow *> mainWindowList;

    foreach(QWidget *widget, qApp->topLevelWidgets()) {
      MainWindow *window = qobject_cast<MainWindow *>( widget );
      if (window && window->isVisible())
        mainWindowList.append(window);
    }

    if (mainWindowList.isEmpty())
      return;

    qSort(mainWindowList.begin(), mainWindowList.end(), windowComparison);

    unsigned int untitledCount = 0;
    ui.menuSettings->addSeparator();
    foreach (MainWindow *widget, mainWindowList) {
      QAction *windowAction = new QAction(widget);
      if (!widget->d->fileName.isEmpty())
        windowAction->setText(QFileInfo(widget->d->fileName).fileName());
      else
        windowAction->setText(tr("Untitled %1").arg(++untitledCount));

      if (widget == this) {
        windowAction->setCheckable(true);
        windowAction->setChecked(true);
      }
      connect(windowAction, SIGNAL(triggered()), widget, SLOT(showAndActivate()));
      ui.menuSettings->addAction(windowAction);
    }
  }

  void MainWindow::about()
  {
    AboutDialog * about = new AboutDialog( this );
    about->show();
  }

  void MainWindow::importFile()
  {
    if (!d->importFile) {
      d->importFile = new ImportDialog(this);
    }
    d->importFile->show();
  }

  // Unfortunately Qt signals/slots doesn't let us pass an arbitrary URL to a slot
  // or we'd have one openURL("string")
  // Instead, we've got a bunch of one-line actions...
  void MainWindow::openTutorialURL() const
  {
    QDesktopServices::openUrl(QUrl("http://avogadro.openmolecules.net/wiki/Tutorials"));
  }

  void MainWindow::openFAQURL() const
  {
    QDesktopServices::openUrl(QUrl("http://avogadro.openmolecules.net/wiki/Avogadro:FAQ"));
  }

  void MainWindow::openWebsiteURL() const
  {
    QDesktopServices::openUrl(QUrl("http://avogadro.openmolecules.net/wiki/"));
  }

  void MainWindow::openReleaseNotesURL() const
  {
    QDesktopServices::openUrl(QUrl( "http://avogadro.openmolecules.net/wiki/Avogadro_" + QString(VERSION) ));
  }

  void MainWindow::openBugURL() const
  {
    QDesktopServices::openUrl(QUrl("http://sourceforge.net/tracker/?group_id=165310&atid=835077"));
  }

  void MainWindow::setView( int index )
  {
    d->glWidget = d->glWidgets.at( index );

    d->enginesStacked->setCurrentIndex( index );
    ui.actionDisplayAxes->setChecked(renderAxes());
    ui.actionDisplayUnitCellAxes->setChecked(renderUnitCellAxes());
    ui.actionDebugInformation->setChecked(renderDebug());
    ui.actionQuickRender->setChecked(quickRender());
  }

  void MainWindow::paste()
  {
    QClipboard *clipboard = QApplication::clipboard();

    const QMimeData *mimeData = NULL;

    bool supportsSelection = clipboard->supportsSelection();

    if ( supportsSelection ) {
      mimeData = clipboard->mimeData( QClipboard::Selection );
    }

    // if we don't support selection, or we failed pasting the selection
    // try from the clipboard
    if ( !supportsSelection || !pasteMimeData(mimeData) )
    {
      mimeData = clipboard->mimeData();
      if(!pasteMimeData(mimeData))
      {
        statusBar()->showMessage( tr( "Unable to paste molecule." ) );
      }
    }
  }

  bool MainWindow::pasteMimeData(const QMimeData *mimeData)
  {
    OBConversion conv;
    OBFormat *pasteFormat = NULL;
    QByteArray text;
    OBMol newMol;

    if ( mimeData->hasFormat( "chemical/x-mdl-molfile" ) ) {
      pasteFormat = conv.FindFormat( "mdl" );

      text = mimeData->data( "chemical/x-mdl-molfile" );
    } else if ( mimeData->hasFormat( "chemical/x-cdx" ) ) {
      pasteFormat = conv.FindFormat( "cdx" );
      text = mimeData->data( "chemical/x-cdx" );
    } else if ( mimeData->hasText() ) {
      pasteFormat = conv.FindFormat( "xyz" );

      text = mimeData->text().toAscii();
    }

    if ( text.length() == 0 )
      return false;

    if ( !pasteFormat || !conv.SetInFormat( pasteFormat ) ) {
      statusBar()->showMessage( tr( "Paste failed (format unavailable)." ), 5000 );
      return false;
    }

    if ( conv.ReadString( &newMol, text.data() )
         && newMol.NumAtoms() != 0 ) {
      vector3 offset; // small offset so that pasted mols don't fall on top
      offset.randomUnitVector();
      offset *= 0.2;

      newMol.Translate(offset);
      Molecule newMolecule;
      newMolecule.setOBMol(&newMol);
      PasteCommand *command = new PasteCommand(d->molecule, newMolecule, d->glWidget);
      d->undoStack->push(command);
      d->toolGroup->setActiveTool("Manipulate"); // set the tool to manipulate, so we can immediate move the selection
    } else {
      return false;
    }
    return true;
  }

  // Helper function -- works for "cut" or "copy"
  // FIXME add parameter to set "Copy" or "Cut" in messages
  QMimeData* MainWindow::prepareClipboardData(PrimitiveList selectedItems)
  {
    QMimeData *mimeData = new QMimeData;
    // we also save an image for copy/paste to office programs, presentations, etc.
    QImage clipboardImage;
    d->glWidget->raise();
    d->glWidget->repaint();
    if (QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
      clipboardImage = d->glWidget->grabFrameBuffer( true );
    } else {
      QPixmap pixmap = QPixmap::grabWindow( d->glWidget->winId() );
      clipboardImage = pixmap.toImage();
    }

    Molecule *moleculeCopy = d->molecule;
    if (!selectedItems.isEmpty()) { // we only want to copy the selected items
      moleculeCopy = new Molecule;
      std::map<unsigned int, unsigned int> AtomMap; // key is from old, value from new
      // copy atoms and create a map of atom indices
      foreach(Primitive *item, selectedItems.subList(Primitive::AtomType)) {
        Atom *selected = moleculeCopy->addAtom();
        *selected = *(static_cast<Atom *>(item));
        AtomMap[item->id()] = selected->id();
      }

      // use the atom map to map bonds
      map<unsigned int, unsigned int>::iterator posBegin, posEnd;
      QList<Bond*> bonds = d->molecule->bonds();
      foreach(const Bond *bond, bonds) {
        posBegin = AtomMap.find(bond->beginAtomId());
        posEnd = AtomMap.find(bond->endAtomId());
        // make sure both bonds are in the map (i.e. selected)
        if ( posBegin != AtomMap.end() && posEnd != AtomMap.end() ) {
          Bond *bondCopy = moleculeCopy->addBond();
          bondCopy->setAtoms(posBegin->second, posEnd->second, bond->order());
        }
      } // end looping over bonds
    } // should now have a copy of our selected fragment

    OBConversion conv;
    // MDL format is used for main copy -- atoms, bonds, chirality
    // supports either 2D or 3D, generic data
    // CML is another option, but not as well tested in Open Babel
    OBFormat *mdlFormat = conv.FindFormat("mdl");
    if (!mdlFormat || !conv.SetOutFormat(mdlFormat)) {
      statusBar()->showMessage( tr( "Copy failed (mdl unavailable)." ), 5000 );
      return NULL; // nothing in it yet
    }

    // write an MDL file first (with bond orders, radicals, etc.)
    // (CML might be better in the future, but this works well now)
    OBMol obmol = moleculeCopy->OBMol();
    string output = conv.WriteString(&obmol);
    QByteArray copyData(output.c_str(), output.length());
    mimeData->setData("chemical/x-mdl-molfile", copyData);

    // we embed the molfile into the image
    // e.g. http://baoilleach.blogspot.com/2007/08/access-embedded-molecular-information.html
    clipboardImage.setText("molfile", copyData);

    // save a canonical SMILES too
    OBFormat *canFormat = conv.FindFormat("can");
    if ( canFormat && conv.SetOutFormat( canFormat ) ) {
      output = conv.WriteString(&obmol);
      copyData = output.c_str();
      clipboardImage.setText("SMILES", copyData);
    }

    // Copy XYZ coordinates to the text selection buffer
    OBFormat *xyzFormat = conv.FindFormat("xyz");
    if ( xyzFormat && conv.SetOutFormat(xyzFormat)) {
      output = conv.WriteString(&obmol);
      copyData = output.c_str();
      mimeData->setText(QString(copyData));
    }

    // need to free our temporary moleculeCopy
    if (!selectedItems.isEmpty()) {
      delete moleculeCopy;
    }

    // save the image to the clipboard too
    mimeData->setImageData(clipboardImage);

    return mimeData;
  }

  void MainWindow::cut()
  {
    QMimeData *mimeData = prepareClipboardData( d->glWidget->selectedPrimitives() );

    if ( mimeData ) {
      CutCommand *command = new CutCommand( d->molecule, mimeData,
          d->glWidget->selectedPrimitives() );
      d->undoStack->push( command );
    }
  }

  void MainWindow::copy()
  {
    QMimeData *mimeData = prepareClipboardData( d->glWidget->selectedPrimitives() );

    if ( mimeData ) {
      QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
    }
  }

  void MainWindow::clear()
  {
    // clear the molecule or a set of atoms
    // has the inteligence to figure out based on the number of selected items
    ClearCommand *command = new ClearCommand( d->molecule,
        d->glWidget->selectedPrimitives() );
    d->undoStack->push( command );
  }

  void MainWindow::selectAll()
  {
    QList<Primitive*> selection;
    QList<Atom*> atoms = d->molecule->atoms();
    foreach(Atom* atom, atoms) {
      selection.append(const_cast<Atom*>(atom));
    }
    QList<Bond*> bonds = d->molecule->bonds();
     foreach(Bond* bond, bonds) {
      selection.append(const_cast<Bond*>(bond));
    }

    d->glWidget->setSelected(selection, true);

    d->glWidget->update();
  }

  void MainWindow::selectNone()
  {
    d->glWidget->clearSelected();
    d->glWidget->update();
  }

  void MainWindow::setPainterQuality( int quality )
  {
    d->glWidget->setQuality( quality );
    d->glWidget->update();
  }

  int MainWindow::painterQuality() const
  {
    return d->glWidget->quality();
  }

  void MainWindow::setFogLevel(int level)
  {
    d->glWidget->setFogLevel(level);
    d->glWidget->update();
  }

  int MainWindow::fogLevel() const
  {
    return d->glWidget->fogLevel();
  }

  void MainWindow::newView()
  {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout( widget );
    layout->setMargin( 0 );
    layout->setSpacing( 6 );
    GLWidget *gl = newGLWidget();
    gl->loadDefaultEngines();
    layout->addWidget(gl);

    QString tabName = tr("View %1").arg( d->centralTab->count()+1 );

    d->centralTab->addTab(widget, tabName);
    ui.actionDisplayAxes->setChecked(gl->renderAxes());
    ui.actionDisplayUnitCellAxes->setChecked(gl->renderUnitCellAxes());
    ui.actionDebugInformation->setChecked(gl->renderDebug());
    ui.actionQuickRender->setChecked(gl->quickRender());
    writeSettings();
  }

  void MainWindow::duplicateView()
  {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout( widget );
    layout->setMargin( 0 );
    layout->setSpacing( 6 );
    GLWidget *gl = newGLWidget();
    layout->addWidget(gl);

    // store current settings
    writeSettings();

    // load settings from current widget
    int currentIndex = d->centralTab->currentIndex();
    QSettings settings;
    settings.beginReadArray("view");
    settings.setArrayIndex(currentIndex);
    gl->readSettings(settings);
    settings.endArray();

    QString tabName = tr("View %1").arg( d->centralTab->count()+1 );

    d->centralTab->addTab( widget, tabName );
    ui.actionCloseView->setEnabled( true );
    ui.actionDisplayAxes->setChecked(gl->renderAxes());
    ui.actionDisplayUnitCellAxes->setChecked(gl->renderUnitCellAxes());
    ui.actionDebugInformation->setChecked(gl->renderDebug());
    ui.actionQuickRender->setChecked(gl->quickRender());

    writeSettings();
  }

  void MainWindow::detachView()
  {
    // Create a new QDialog and layout
//    QDialog *dialog = new QDialog(this);
//    QVBoxLayout *layout = new QVBoxLayout(dialog);
//    layout->setMargin( 0 );
//    layout->setSpacing( 6 );

    // Get the GLWidget of the current view, close in in the tabs
    QWidget *widget = d->centralTab->currentWidget();
    foreach(QObject *object, widget->children()) {
      GLWidget *glWidget = qobject_cast<GLWidget *>(object);
      if (glWidget) {
        int index = d->centralTab->currentIndex();
        d->centralTab->removeTab(index);

        for (int count=d->centralTab->count(); index < count; index++) {
          d->centralTab->setTabText(index, tr( "View %1" )
                                           .arg( index + 1) );
        }
        ui.actionCloseView->setEnabled( d->centralTab->count() != 1 );
        // Set the GLWidget as the main widget in the dialog
        //layout->addWidget(glWidget);
        GLGraphicsView *view = new GLGraphicsView(glWidget, 0);
        view->setWindowTitle(tr("Avogadro: Detached View"));
        view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        view->setScene(new QGraphicsScene);
        view->scene()->addText("Avogadro GLGraphicsView");
        view->show();

      }
    }
  }

  void MainWindow::closeView()
  {
    QWidget *widget = d->centralTab->currentWidget();
    foreach( QObject *object, widget->children() ) {
      GLWidget *glWidget = qobject_cast<GLWidget *>( object );
      if ( glWidget ) {
        int index = d->centralTab->currentIndex();
        d->centralTab->removeTab( index );

        // delete the engines list for this GLWidget
        QWidget *widget = d->enginesStacked->widget( index );
        d->enginesStacked->removeWidget( widget );
        delete widget;

        for ( int count=d->centralTab->count(); index < count; ++index ) {
          d->centralTab->setTabText(index, tr("View %1").arg( index + 1) );
        }
        d->glWidgets.removeAll( glWidget );
        delete glWidget;
        ui.actionCloseView->setEnabled( d->centralTab->count() != 1 );
      }
    }

    setView( d->centralTab->currentIndex() );

    writeSettings();
  }

  void MainWindow::centerStep()
  {
    Camera *camera = d->glWidget->camera();
    if(!camera)
    {
      d->centerTimer->deleteLater();
      d->centerTimer = 0;
      d->centerTime = 0;
      return;
    }


    // calculate elapsed time
    d->centerTime += 10;
    int elapsedTime = d->centerTime;

    if(elapsedTime > d->rotationTime)
      elapsedTime = d->rotationTime;

    // make sure we don't divide by zero (0)
    if(d->rotationTime != 0) {
      double x = (elapsedTime * (M_PI) / d->rotationTime);
      double r = (cos(x-M_PI)+1)/2;

      camera->modelview().linear() = d->startOrientation.slerp(r,d->endOrientation).toRotationMatrix();
      camera->modelview().translation() = d->startTrans + d->deltaTrans * r;
    }

    if(elapsedTime >= d->rotationTime) {
      d->centerTimer->deleteLater();
      d->centerTimer = 0;
      d->centerTime = 0;
      //cout << "Final Translation: " << camera->modelview().translationVector() << endl;
      //cout << "Final Linear: " << camera->modelview().linearComponent() << endl << endl;
    }

    d->glWidget->update();
  }

  void MainWindow::centerView()
  {
    // do nothing if there is a timer running
    if(d->centerTimer)
      return;

    Camera * camera = d->glWidget->camera();
    if(!camera)
      return;

    // no need to animate when there are no atoms
    if(d->molecule->numAtoms() == 0)  {
      camera->translate( d->glWidget->center() - Vector3d( 0.0, 0.0, 20.0 ) );
      d->glWidget->update();
      return;
    }

    // determine our goal matrix
    Matrix3d linearGoal;
    linearGoal.row(2) = d->glWidget->normalVector();
    linearGoal.row(0) = linearGoal.row(2).unitOrthogonal();
    linearGoal.row(1) = linearGoal.row(2).cross(linearGoal.row(0));

    // calculate the translation matrix
    Transform3d goal(linearGoal);

    goal.pretranslate(- 3.0 * (d->glWidget->radius() + CAMERA_NEAR_DISTANCE) * Vector3d::UnitZ());

    // Support centering on a selection
    QList<Primitive*> selectedAtoms = d->glWidget->selectedPrimitives().subList(Primitive::AtomType);
    if (selectedAtoms.isEmpty()) { // no selected atoms, we want the global center
      goal.translate( - d->glWidget->center() );
    } else {
      // Calculate the centroid of the selection
      Vector3d selectedCenter(0.0, 0.0, 0.0);
      foreach(Primitive *item, selectedAtoms) {
        // Atom::pos() returns a pointer to the position
        selectedCenter += *(static_cast<Atom*>(item)->pos());
      }
      selectedCenter /= double(selectedAtoms.size());
      goal.translate( -selectedCenter);
    }

    // if smooth transitions are disabled, center now and return
    if( !d->molecule->numAtoms() >= 1000 ) {
      camera->setModelview(goal);
      d->glWidget->update();
      return;
    }

    d->startTrans = camera->modelview().translation();
    d->deltaTrans = goal.translation() - d->startTrans;

    d->startOrientation = camera->modelview().linear();
    d->endOrientation = goal.linear();

    // Use QTimer for smooth transitions
    d->centerTime = 0;

    // use the rotation angle between the two orientations to calculate our animation time
    double m = AngleAxisd(d->startOrientation.inverse() * d->endOrientation).angle();
    d->rotationTime = int(m*300.0);

    if(d->rotationTime < 300 && d->deltaTrans.squaredNorm() > 1)
      d->rotationTime = 500;

    // make sure we need to rotate
    if(d->rotationTime > 0) {
      d->centerTimer = new QTimer();
      connect(d->centerTimer, SIGNAL(timeout()),
          this, SLOT(centerStep()));
      d->centerTimer->start(10);
    }
  }

  void MainWindow::showAndActivate()
  {
    setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    raise();
  }

  void MainWindow::fullScreen()
  {
    if ( !this->isFullScreen() ) {
      ui.actionFullScreen->setText( tr( "Normal Size" ) );
      d->fileToolbar = ui.fileToolBar->isVisible();
      d->statusBar = statusBar()->isVisible();
      ui.fileToolBar->hide();
      statusBar()->hide();
      // From KDE: avoid Full Screen
      //      this->showFullScreen();
      this->setWindowState(this->windowState() | Qt::WindowFullScreen);
    } else {
      // From KDE: Krazy -- showNormal() is not the inverse of Full Screen
      //      this->showNormal();
      this->setWindowState(this->windowState() & ~Qt::WindowFullScreen);
      ui.actionFullScreen->setText( tr( "Full Screen" ) );

      ui.fileToolBar->setVisible(d->fileToolbar);
      statusBar()->setVisible(d->statusBar);
    }
  }

  void MainWindow::showSettingsDialog()
  {
    if ( !d->settingsDialog ) {
      d->settingsDialog = new SettingsDialog( this );
      // Add the Plugin settings
      //QWidget *pluginSettings = d->pluginManager.settingsWidget();
      QWidget *pluginSettings = new PluginSettings;
      connect(pluginSettings, SIGNAL(reloadPlugins()), &d->pluginManager, SLOT(reload()));
      connect(&d->pluginManager, SIGNAL(reloadPlugins()), pluginSettings, SLOT(loadValues()));
      d->settingsDialog->insertWidget(1, pluginSettings);
      // Add the Project Tree Editor
      ProjectTreeEditor *projectTreeEditor = new ProjectTreeEditor;
      connect( projectTreeEditor, SIGNAL( structureChanged() ), this, SLOT( setupProjectTree() ) );
      d->settingsDialog->insertWidget(2, projectTreeEditor);
    }
    d->settingsDialog->show();
  }

  void MainWindow::setBackgroundColor()
  {
    QColor color = QColorDialog::getColor(d->glWidget->background(), this);
    if (color.isValid()) {
      d->glWidget->setBackground(color);
      d->glWidget->update();
    }
  }

  void MainWindow::setTool( Tool *tool )
  {
    d->toolSettingsDock->setWindowTitle(tool->settingsTitle());
    if ( tool->settingsWidget() ) {
      d->toolSettingsStacked->setCurrentWidget( tool->settingsWidget() );
    } else {
      d->toolSettingsStacked->setCurrentIndex( 0 );
    }
  }

  void MainWindow::connectUi()
  {
    // We have duplicate actions for the menus and the toolbars for Mac
    // This way we can disable the menus when all windows are closed
    // and disable menu icons (without disabling the toolbar icons)
    connect( ui.actionNew, SIGNAL( triggered() ), this, SLOT( newFile() ) );
    connect( ui.actionNewTool, SIGNAL( triggered() ), this, SLOT( newFile() ) );
    connect( ui.actionOpen, SIGNAL( triggered() ), this, SLOT( openFile() ) );
    connect( ui.actionOpenTool, SIGNAL( triggered() ), this, SLOT( openFile() ) );
#ifdef Q_WS_MAC
    connect( ui.actionClose, SIGNAL( triggered() ), this, SLOT( close() ) );
    connect( ui.actionCloseTool, SIGNAL( triggered() ), this, SLOT( close() ) );
#else
    connect( ui.actionClose, SIGNAL( triggered() ),
             this, SLOT( closeFile() ) );
    connect( ui.actionCloseTool, SIGNAL( triggered() ),
             this, SLOT( closeFile() ) );
#endif
    connect( ui.actionSave, SIGNAL( triggered() ), this, SLOT( save() ) );
    connect( ui.actionSaveTool, SIGNAL( triggered() ), this, SLOT( save() ) );
    connect( ui.actionSaveAs, SIGNAL( triggered() ), this, SLOT( saveAs() ) );
    connect( ui.actionRevert, SIGNAL( triggered() ), this, SLOT( revert() ) );
    connect( ui.actionImport_File, SIGNAL( triggered() ),
             this, SLOT( importFile() ) );
    connect( ui.actionExportGraphics, SIGNAL( triggered() ),
             this, SLOT( exportGraphics() ) );
    connect( ui.actionExportGL2PS, SIGNAL(triggered()), this, SLOT(exportGL2PS()));
#ifdef Q_WS_MAC
    connect( ui.actionQuit, SIGNAL( triggered() ), this, SLOT( macQuit() ) );
    connect( ui.actionQuitTool, SIGNAL( triggered() ), this, SLOT( macQuit() ) );
#else
    connect( ui.actionQuit, SIGNAL( triggered() ), this, SLOT( close() ) );
    connect( ui.actionQuitTool, SIGNAL( triggered() ), this, SLOT( close() ) );
#endif

    connect( ui.actionClearRecent, SIGNAL( triggered() ),
             this, SLOT( clearRecentFiles() ) );

    connect( d->undoStack, SIGNAL( cleanChanged( bool ) ),
             this, SLOT( undoStackClean( bool ) ) );

    connect( ui.actionCut, SIGNAL( triggered() ), this, SLOT( cut() ) );
    connect( ui.actionCopy, SIGNAL( triggered() ), this, SLOT( copy() ) );
    connect( ui.actionPaste, SIGNAL( triggered() ), this, SLOT( paste() ) );
    connect( ui.actionClear, SIGNAL( triggered() ), this, SLOT( clear() ) );

    // By default, the UI template defines backspace as the shortcut
    // We'll add control-backspace (which was the default in prev. versions)
    QList<QKeySequence> clearShortcuts =  ui.actionClear->shortcuts();
    clearShortcuts << QKeySequence(tr("Ctrl+Backspace"));
    ui.actionClear->setShortcuts(clearShortcuts);

    connect( ui.actionSelect_All, SIGNAL( triggered() ),
             this, SLOT( selectAll() ) );
    connect( ui.actionSelect_None, SIGNAL( triggered() ),
             this, SLOT( selectNone() ) );

    connect( ui.actionNewView, SIGNAL( triggered() ),
             this, SLOT( newView() ) );
    connect( ui.actionDuplicateView, SIGNAL( triggered() ),
            this, SLOT( duplicateView() ) );
    connect(ui.actionDetachView, SIGNAL(triggered()),
            this, SLOT(detachView()));
    connect( ui.actionCloseView, SIGNAL( triggered() ),
             this, SLOT( closeView() ) );
    connect( ui.actionCenter, SIGNAL( triggered() ),
             this, SLOT( centerView() ) );
    connect( ui.actionFullScreen, SIGNAL( triggered() ),
             this, SLOT( fullScreen() ) );
    connect( ui.actionSetBackgroundColor, SIGNAL( triggered() ),
             this, SLOT( setBackgroundColor() ) );
    connect(ui.actionDisplayAxes, SIGNAL(triggered(bool)),
            this, SLOT(setRenderAxes(bool)));
    connect(ui.actionDisplayUnitCellAxes, SIGNAL(triggered(bool)),
            this, SLOT(setRenderUnitCellAxes(bool)));
    connect(ui.actionDebugInformation, SIGNAL(triggered(bool)),
            this, SLOT(setRenderDebug(bool)));
    connect(ui.actionQuickRender, SIGNAL(triggered(bool)),
            this, SLOT(setQuickRender(bool)));
    connect(ui.actionAllMolecules, SIGNAL(triggered(bool)),
            this, SLOT(showAllMolecules(bool)));

    connect( ui.actionAbout, SIGNAL( triggered() ), this, SLOT( about() ) );

    connect( d->centralTab, SIGNAL( currentChanged( int ) ),
             this, SLOT( setView( int ) ) );

    connect( ui.configureAvogadroAction, SIGNAL( triggered() ),
        this, SLOT( showSettingsDialog() ) );

    connect( ui.projectTreeView, SIGNAL(activated(const QModelIndex&)),
        this, SLOT(projectItemActivated(const QModelIndex&)));

    connect( ui.actionTutorials, SIGNAL( triggered() ),
             this, SLOT( openTutorialURL() ));
    connect( ui.actionFAQ, SIGNAL( triggered() ),
             this, SLOT( openFAQURL() ) );
    connect( ui.actionRelease_Notes, SIGNAL( triggered() ),
             this, SLOT( openReleaseNotesURL() ));
    connect( ui.actionAvogadro_Website, SIGNAL( triggered() ),
             this, SLOT( openWebsiteURL() ) );
    connect( ui.actionReport_a_Bug, SIGNAL( triggered() ),
             this, SLOT( openBugURL() ) );


    connect( d->toolGroup, SIGNAL( toolActivated( Tool * ) ),
             this, SLOT( setTool( Tool * ) ) );
    connect( this, SIGNAL( moleculeChanged( Molecule * ) ), d->toolGroup,
             SLOT( setMolecule( Molecule * ) ) );

  }

  void MainWindow::setMolecule( Molecule *molecule )
  {
    if ( d->molecule ) {
      disconnect( d->molecule, 0, this, 0 );
      d->molecule->deleteLater();
    }

    d->undoStack->clear();

    d->molecule = molecule;

    QString newFileName = molecule->fileName();
    setFileName(newFileName);

    if (newFileName.isEmpty()) {
      setWindowFilePath(tr("untitled") + ".cml");
    }

    emit moleculeChanged(molecule);

    connect( d->molecule, SIGNAL( primitiveAdded( Primitive * ) ),
             this, SLOT( documentWasModified() ) );
    connect( d->molecule, SIGNAL( primitiveUpdated( Primitive * ) ),
             this, SLOT( documentWasModified() ) );
    connect( d->molecule, SIGNAL( primitiveRemoved( Primitive * ) ),
             this, SLOT( documentWasModified() ) );

    setWindowModified( false );
  }

  void MainWindow::setMolecule(Molecule *molecule, int options)
  {
    if (d->molecule && options & Extension::DeleteOld) {
      disconnect(d->molecule, 0, this, 0);
      d->molecule->deleteLater();
      qDebug() << "Old molecule deleted...";
    }
    else if (d->molecule) {
      disconnect(d->molecule, 0, this, 0);
      qDebug() << "Old molecule replaced...";
    }

    d->undoStack->clear();

    d->molecule = molecule;

    QString newFileName = molecule->fileName();
    setFileName(newFileName);

    if (newFileName.isEmpty())
      setWindowFilePath(tr("untitled") + ".cml");

    emit moleculeChanged(molecule);

    connect( d->molecule, SIGNAL( primitiveAdded( Primitive * ) ),
             this, SLOT( documentWasModified() ) );
    connect( d->molecule, SIGNAL( primitiveUpdated( Primitive * ) ),
             this, SLOT( documentWasModified() ) );
    connect( d->molecule, SIGNAL( primitiveRemoved( Primitive * ) ),
             this, SLOT( documentWasModified() ) );
    setWindowModified(false);
  }

  void MainWindow::setupProjectTree()
  {
    ProjectTreeModel *model = qobject_cast<ProjectTreeModel*>(ui.projectTreeView->model());

    ui.projectTreeView->setModel(new ProjectTreeModel( d->glWidget, this ));

    if (model)
      delete model;
  }

  void MainWindow::projectItemActivated(const QModelIndex& index)
  {
    // select the new primitives
    ProjectTreeModel *model = qobject_cast<ProjectTreeModel*>(ui.projectTreeView->model());
    if (!model)
      return;

    ProjectTreeItem *projectItem = model->item(index);
    if (projectItem) {
      d->glWidget->clearSelected();
      d->glWidget->setSelected(projectItem->primitives(), true);
      d->glWidget->update();
    }
  }

  Molecule *MainWindow::molecule() const
  {
    return d->molecule;
  }

  void MainWindow::setFileName( const QString &fileName )
  {
    if ( fileName.isEmpty() ) {
      d->fileName.clear();
      setWindowTitle( tr( "[*]Avogadro" , "[*] indicates that this is a modified view...." ) );
    } else {
      QFileInfo fileInfo(fileName);
      d->fileName = fileInfo.canonicalFilePath();
      d->fileDialogPath = fileInfo.absolutePath();
      setWindowTitle( tr( "%1[*] - %2", "Window title: %1 is file name, %2 is Avogadro" ).arg( fileInfo.fileName() )
          .arg( tr( "Avogadro" ) ) );

      QSettings settings; // already set up properly via main.cpp
      QStringList files = settings.value( "recentFileList" ).toStringList();
      files.removeAll( fileName );
      files.prepend( fileName );
      while ( files.size() > maxRecentFiles )
        files.removeLast();

      settings.setValue( "recentFileList", files );

      // Set the fileName for the actual molecule too
      if (d->molecule) {
        d->molecule->setFileName(fileInfo.absoluteFilePath());
      }
    }

    foreach( QWidget *widget, QApplication::topLevelWidgets() ) {
      MainWindow *mainWin = qobject_cast<MainWindow *>( widget );
      if ( mainWin )
        mainWin->updateRecentFileActions();
    }
  }

  void MainWindow::updateRecentFileActions()
  {
    QSettings settings; // set up project and program properly in main.cpp
    QStringList files = settings.value( "recentFileList" ).toStringList();

    int numRecentFiles = qMin( files.size(), ( int )maxRecentFiles );

    for ( int i = 0; i < numRecentFiles; ++i ) {
      // Skip files that don't exist...
      if (!QFile(QFileInfo(files[i]).absoluteFilePath()).exists()) {
        continue;
      }
      d->actionRecentFile[i]->setText( QFileInfo(files[i]).fileName() );
      d->actionRecentFile[i]->setData( files[i] );
      d->actionRecentFile[i]->setVisible( true );
    }
    for ( int j = numRecentFiles; j < maxRecentFiles; ++j )
      d->actionRecentFile[j]->setVisible( false );

    //     ui.actionSeparator->setVisible(numRecentFiles > 0);
  }

  MainWindow *MainWindow::findMainWindow( const QString &fileName )
  {
    QString canonicalFilePath = QFileInfo( fileName ).canonicalFilePath();

    // If the canonical file path is empty then the file doesn't exist
    if (canonicalFilePath.isEmpty())
      return 0;

    foreach(QWidget *widget, qApp->topLevelWidgets()) {
      MainWindow *window = qobject_cast<MainWindow *>( widget );
      if (window && window->d->fileName == canonicalFilePath)
        return window;
    }
    return 0;
  }

  void MainWindow::readSettings()
  {
    QSettings settings;
    // Check if the config file version exists and whether to the saved settings
    if (settings.value("ConfigVersion", 0) != m_configFileVersion) {
      settings.clear();
    }

    // Only remember a window if it is the first one - others will be offset
    if (getMainWindowCount() == 1) {
      QPoint originalPosition = pos();
      QPoint newPosition = settings.value("pos", QPoint(200, 200)).toPoint();

      // We'll try moving the window. If it moves off-screen, we'll move it back
      // This solves PR#1903437
      move(newPosition);
      QDesktopWidget desktop;
      if (desktop.screenNumber(this) == -1) // it's not on a screen
        move(originalPosition);
    }

    QSize size = settings.value( "size", QSize(720,540) ).toSize();
    resize( size );

    d->fileDialogPath = settings.value("openDialogPath").toString();

    QByteArray ba = settings.value( "state" ).toByteArray();
    if(!ba.isEmpty())
    {
      restoreState(ba);
    } else {
//      tabifyDockWidget(ui.enginesDock, ui.engineConfigurationDock);
//      tabifyDockWidget(ui.enginesDock, ui.enginePrimitivesDock);
      ui.enginesDock->raise();
    }

    settings.beginGroup("tools");
    d->toolGroup->readSettings(settings);
    settings.endGroup();

    settings.beginGroup("extensions");
    foreach(Extension *extension, d->pluginManager.extensions(this)) {
      extension->readSettings(settings);
    }
    settings.endGroup();

    settings.beginGroup("colors");
    foreach(Color *color, d->pluginManager.colors(this)) {
      color->readSettings(settings);
    }
    settings.endGroup();

    int count = settings.beginReadArray("view");
    if(count < 1)
      count = 1;

    for(int i = 0; i<count; i++) {
      settings.setArrayIndex(i);
      QWidget *widget = new QWidget();
      QVBoxLayout *layout = new QVBoxLayout( widget );
      layout->setMargin( 0 );
      layout->setSpacing( 6 );
      GLWidget *gl = newGLWidget();
      layout->addWidget(gl);

      setupProjectTree();

      QString tabName = tr("View %1").arg( i+1 );
      d->centralTab->addTab(widget, tabName);

      gl->readSettings(settings);
    }
    settings.endArray();

    // Set the view conditions for the initial view
    ui.actionDisplayAxes->setChecked(renderAxes());
    ui.actionDisplayUnitCellAxes->setChecked(renderUnitCellAxes());
    ui.actionDebugInformation->setChecked(renderDebug());
    ui.actionQuickRender->setChecked(quickRender());

    ui.actionCloseView->setEnabled(count > 1);

#ifdef ENABLE_UPDATE_CHECKER
    // Load the updated version configuration settings and then run it
    m_updateCheck->readSettings(settings);
    m_updateCheck->checkForUpdates();
#endif
  }

  void MainWindow::writeSettings()
  {
    QSettings settings;
    // Save the version of the config file - bump if necessary
    settings.setValue("ConfigVersion", m_configFileVersion);
    settings.setValue( "pos", pos() );

    settings.setValue( "size", size() );
    settings.setValue( "state", saveState() );

    settings.setValue("openDialogPath", d->fileDialogPath);
    settings.setValue( "enginesDock", ui.enginesDock->saveGeometry());

    // save the views
    settings.beginWriteArray("view");
    int count = d->glWidgets.size();
    for(int i=0; i<count; i++)
    {
      settings.setArrayIndex(i);
      d->glWidgets.at(i)->writeSettings(settings);
    }
    settings.endArray();

    // write the settings for every tool
    settings.beginGroup("tools");
    d->toolGroup->writeSettings(settings);
    settings.endGroup();

    settings.beginGroup("extensions");
    foreach(Extension *extension, d->pluginManager.extensions(this)) {
      extension->writeSettings(settings);
    }
    settings.endGroup();

    settings.beginGroup("colors");
    foreach(Color *color, d->pluginManager.colors(this)) {
      color->writeSettings(settings);
    }
    settings.endGroup();

#ifdef ENABLE_UPDATE_CHECKER
    // Write the updated version configuration settings
    m_updateCheck->writeSettings(settings);
#endif
  }

  void MainWindow::addActionsToMenu(Extension *extension)
  {
    QList<QAction *> actions = extension->actions();

    foreach(QAction *action, actions)
    {
      // Here's the fun part, we go customize our menus
      // Add these actions to the menu described by the menuPath
      QString menuPathString = extension->menuPath(action);
      QMenu *path = NULL;

      if ( menuPathString.size() ) {
        QStringList menuPath = menuPathString.split( '>' );
        // Root menus are a special case, we need to check menuBar()
        foreach( QAction *menu, menuBar()->actions() ) {
          if ( menu->text() == menuPath.at( 0 ) ) {
            path = menu->menu();
            break;
          }
        }

        if ( !path ) {
          // Gotta add a new root menu
          path = new QMenu(menuPath.at( 0 ));
          menuBar()->insertMenu( ui.menuSettings->menuAction(), path);
        }

        // Now handle submenus
        if ( menuPath.size() > 1 ) {
          QMenu *nextPath = NULL;

          // Go through each submenu level, find the match
          // and update the "path" pointer
          for ( int i = 1; i < menuPath.size(); ++i ) {

            foreach( QAction *menu, path->actions() ) {
              if ( menu->text() == menuPath.at( i ) ) {
                nextPath = menu->menu();
                break;
              }
            } // end checking menu items

            if ( !nextPath ) {
              // add a new submenu
              nextPath = path->addMenu( menuPath.at( i ) );
            }

            path = nextPath;
          } // end looping through menuPath
        } // endif
      }

      if(!path) {
        path = ui.menuExtensions;
      }

      path->addAction( action );
      connect( action, SIGNAL( triggered() ), this, SLOT( actionTriggered() ) );
    }
  }

  void MainWindow::loadExtensions()
  {
    foreach(Extension *extension, d->pluginManager.extensions(this)) {
      addActionsToMenu(extension);

      QDockWidget *dockWidget = extension->dockWidget();
      if(dockWidget) {
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
        dockWidget->hide();
        ui.menuToolbars->addAction(dockWidget->toggleViewAction());
      }

      connect(this, SIGNAL(moleculeChanged(Molecule*)),
              extension, SLOT(setMolecule(Molecule*)));
      // When loading a molecule with an already open window with another
      // molecule this signal is never triggered. If we already have a
      // molecule at the point of loading then set it for the extension
      if (d->molecule)
        extension->setMolecule(d->molecule);

      connect(extension, SIGNAL( message(QString)),
            d->messagesText, SLOT(append(QString)));
      connect(extension, SIGNAL( actionsChanged(Extension*) ),
            this, SLOT(addActionsToMenu(Extension*)));
      connect(extension, SIGNAL(moleculeChanged(Molecule *, int)),
            this, SLOT(setMolecule(Molecule *, int)));
      connect(extension, SIGNAL(performCommand(QUndoCommand *)),
            this, SLOT(performCommand(QUndoCommand *)));
    }
  }

  void MainWindow::actionTriggered()
  {
    QAction *action = qobject_cast<QAction *>( sender() );
    if ( action ) {
      Extension *extension = qobject_cast<Extension *>( action->parent() );

      QUndoCommand *command = 0;
      command = extension->performAction( action, d->glWidget);

      if ( command ) {
        d->undoStack->push( command );
      }
    }
  }

  void MainWindow::performCommand(QUndoCommand *command)
  {
    if ( command )
      d->undoStack->push( command );
  }

  void MainWindow::hideMainWindowMac()
  {
    // First remove the last menu item on the "Window" menu
    // i.e., the action which refers to this window
    QAction *lastAction = ui.menuSettings->actions().last();
    ui.menuSettings->removeAction(lastAction);
    ui.menuSettings->actions().last(); // and last separator
    ui.menuSettings->removeAction(lastAction);

    d->menuItemStatus.clear();
    QVector<bool> status;

    // Save the enabled state of every menu item
    foreach( QAction *menu, menuBar()->actions() ) {
      status.clear();
      foreach( QAction *menuItem, menu->menu()->actions() ) {
        status.append( menuItem->isEnabled() );
      }
      d->menuItemStatus.append( status );
    }
    // Now disable them -- do this after saving status, since some actions may be
    // in multiple menus, e.g. "Select All"
    foreach( QAction *menu, menuBar()->actions() ) {
      foreach( QAction *menuItem, menu->menu()->actions() ) {
        menuItem->setEnabled(false);
      }
    }

    // Now enable key menu items -- new, open, open recent, quit, etc.
    ui.actionAbout->setEnabled( true );
    ui.actionNew->setEnabled( true );
    ui.actionOpen->setEnabled( true );
    ui.menuOpenRecent->menuAction()->setEnabled( true );
    ui.actionQuit->setEnabled( true );

    // Clear the molecule
    loadFile();
    hide();
    updateWindowMenu();
  }

  void MainWindow::showMainWindowMac()
  {
    if (d->menuItemStatus.empty())
      return; // we're initializing the application, not showing after a hide calls

    // Set the status of menu items to what we saved with hideMainWindowMac()
    unsigned int menuIndex = 0;
    unsigned int itemIndex = 0;
    foreach( QAction *menu, menuBar()->actions() ) {
      itemIndex = 0;
      foreach( QAction *menuItem, menu->menu()->actions() ) {
        menuItem->setEnabled( d->menuItemStatus[menuIndex][itemIndex] );
        itemIndex++;
      }
      menuIndex++;
    }

    // make sure the camera is set to the default -- reported by Ian Davis
    centerView();

    // Now show the window and raise it
    show();
    raise();
    activateWindow();
  }

  GLWidget *MainWindow::newGLWidget()
  {
    GLWidget *gl = 0;
    if(!d->glWidget)
    {
      gl = new GLWidget(this);
      d->glWidget = gl;
    }
    else
    {
      gl = new GLWidget( d->glWidget->format(), this, d->glWidget );
    }

    connect( this, SIGNAL( moleculeChanged( Molecule * ) ),
             gl, SLOT( setMolecule( Molecule * ) ) );

    gl->setMolecule(d->molecule);
    gl->setObjectName(QString::fromUtf8("glWidget"));
    gl->setUndoStack( d->undoStack );
    gl->setToolGroup( d->toolGroup );
    d->glWidgets.append(gl);

    // engine list wiget contains all the buttons too
    QWidget *engineListWidget = new QWidget(ui.enginesWidget);
    QVBoxLayout *vlayout = new QVBoxLayout(engineListWidget);

    EngineListView *engineListView = new EngineListView( gl, engineListWidget );
    vlayout->addWidget(engineListView);

    // buttons for engines
    // First the settings button
    QHBoxLayout *hlayout = new QHBoxLayout();
//    hlayout->addStretch(1);
    QPushButton *engineSettingsButton = new QPushButton(tr("Settings..."), engineListWidget);
    engineSettingsButton->setEnabled(false);
    hlayout->addWidget(engineSettingsButton);
    connect(engineSettingsButton, SIGNAL(clicked()), this, SLOT(engineSettingsClicked()));
    connect(this, SIGNAL(enableEngineSettingsButton(bool)), engineSettingsButton, SLOT(setEnabled(bool)));
//    hlayout->addStretch(1);
    vlayout->addLayout(hlayout);

    // Then a row of add, duplicate, remove
    hlayout = new QHBoxLayout();
    // add
    QPushButton *addEngineButton = new QPushButton(tr("Add"), engineListWidget);
    hlayout->addWidget(addEngineButton);
    connect(addEngineButton, SIGNAL(clicked()), this, SLOT(addEngineClicked()));

    // duplicate
    QPushButton *duplicateEngineButton = new QPushButton(tr("Duplicate"), engineListWidget);
    hlayout->addWidget(duplicateEngineButton);
    connect(duplicateEngineButton, SIGNAL(clicked()), this, SLOT(duplicateEngineClicked()));

    // remove
    QPushButton *removeEngineButton = new QPushButton(tr("Remove"), engineListWidget);
    hlayout->addWidget(removeEngineButton);
    connect(removeEngineButton, SIGNAL(clicked()), this, SLOT(removeEngineClicked()));
    vlayout->addLayout(hlayout);

    d->enginesStacked->addWidget( engineListWidget );

    // stacked configurations
//    QStackedWidget *stacked = new QStackedWidget(ui.engineConfigurationWidget);
    // 0 position is a blank configuration
//    stacked->addWidget(new QWidget());
//    d->engineConfigurationStacked->addWidget(stacked);

//    EnginePrimitivesWidget *primitivesWidget =
//      new EnginePrimitivesWidget(gl, ui.enginePrimitivesWidget);
//    d->enginePrimitivesStacked->addWidget(primitivesWidget);

    connect( engineListView, SIGNAL( clicked( Engine * ) ),
        this, SLOT( engineClicked( Engine * ) ) );

//    connect( engineListView, SIGNAL( clicked( Engine * ) ),
//        primitivesWidget, SLOT( setEngine( Engine * ) ) );

    // Warn the user if no engines or tools are loaded
    int nEngines = d->pluginManager.factories(Plugin::EngineType).size() - 1;
    int nTools = d->pluginManager.factories(Plugin::ToolType).size();
    QString error;
    if(!nEngines && !nTools)
      error = tr("No tools or engines loaded.");
    else if(!nEngines)
      error = tr("No engines loaded.");
    else if(!nTools)
      error = tr("No tools loaded.");
    error += tr(" It is unlikely this application will function correctly. Please correct this error.");
    // Display a warning dialog if we haven't loaded any tools or engines
    if(!nEngines || !nTools)
      QMessageBox::warning(this, tr("Avogadro"), error);

    return gl;
  }

  void MainWindow::engineSettingsClicked()
  {
    if (!d->currentSelectedEngine)
      return;
    Engine *selectedEngine = d->currentSelectedEngine;

    QWidget *settingsWindow = d->engineSettingsWindows.value(selectedEngine);

    if(settingsWindow) {
      settingsWindow->show();
      return;
    }
    settingsWindow = new QWidget(this, Qt::Dialog);
    settingsWindow->setWindowTitle(selectedEngine->name() + ' ' + tr("Settings"));
    QVBoxLayout *layout = new QVBoxLayout(settingsWindow);

    QWidget *settingsWidget = selectedEngine->settingsWidget();
    QTabWidget *settingsTabs; // may be unused

    //Check to see what tabs are needed (if any)
    bool objectsTab = (selectedEngine->primitiveTypes() & (Engine::Atoms | Engine::Bonds));
    bool colorsTab = (selectedEngine->colorTypes() & Engine::ColorPlugins);
    // do we even need tabs for the window (e.g., axes engine with no objects or colors)
    bool multipleTabsNeeded = (objectsTab || colorsTab);

    if (!multipleTabsNeeded) {
      layout->addWidget(settingsWidget);
    }
    else {
      settingsTabs = new QTabWidget(settingsWindow);
      settingsTabs->addTab(settingsWidget, tr("Settings"));

      if (objectsTab) {
        EnginePrimitivesWidget *primitivesWidget =
            new EnginePrimitivesWidget(d->glWidget, settingsWindow);
        primitivesWidget->setEngine(selectedEngine);
        settingsTabs->addTab(primitivesWidget, tr("Objects"));
      }

      if (colorsTab) {
        EngineColorsWidget *colorsWidget = new EngineColorsWidget(&(d->pluginManager), settingsWindow);
        colorsWidget->setEngine(selectedEngine);
        settingsTabs->addTab(colorsWidget, tr("Colors"));
      }
      layout->addWidget(settingsTabs);
    }

    connect(settingsWidget, SIGNAL(destroyed()), settingsWindow, SLOT(deleteLater()));

    d->engineSettingsWindows[selectedEngine] = settingsWindow;
    settingsWindow->setLayout(layout);
    settingsWindow->show();
  }

  void MainWindow::addEngineClicked()
  {
    Engine *engine =  AddEngineDialog::getEngine(this, d->pluginManager.factories(Plugin::EngineType));
    if(engine) {
      PrimitiveList p = d->glWidget->selectedPrimitives();
      if(p.size())
        engine->setPrimitives(p);
      d->glWidget->addEngine(engine);
    }
  }

  void MainWindow::duplicateEngineClicked()
  {
    // get the current widget for the engines
    QWidget *widget = d->enginesStacked->currentWidget();

    foreach(QObject *object, widget->children())
    {
      // Since our EngineListViews are contained in a parent QWidget
      // we have to search our children for the actual EngineListView.
      EngineListView *engineListView;
      if( object->isWidgetType() &&
          (engineListView = qobject_cast<EngineListView *>(object)) )
      {
        Engine *engine = engineListView->selectedEngine();

        if(engine) {
          Engine *newEngine = engine->clone();
          PrimitiveList list = d->glWidget->selectedPrimitives();
          if(list.size()) {
            newEngine->setPrimitives(list);
          }
          newEngine->setAlias(newEngine->alias() + tr(" copy"));
          d->glWidget->addEngine(newEngine);
        }
        break;
      }
    }
  }

  void MainWindow::removeEngineClicked()
  {
    QWidget *widget = d->enginesStacked->currentWidget();
    foreach(QObject *object, widget->children()) {
      EngineListView *engineListView;
      if( object->isWidgetType() &&
          (engineListView = qobject_cast<EngineListView *>(object)) )
      {
        Engine *engine = engineListView->selectedEngine();

        if(engine) {
          d->glWidget->removeEngine(engine);
          d->engineSettingsWindows.remove(engine);
          emit enableEngineSettingsButton(false);
        }
        break;
      }
    }

  }

  void MainWindow::engineClicked(Engine *engine)
  {
    if (!engine)
      return;

    d->currentSelectedEngine = engine;
    // If we have a non-null widget, enable the settings button
    emit enableEngineSettingsButton(engine->settingsWidget() != NULL);
  }

  void MainWindow::toggleToolSettingsDock()
  {
    d->toolSettingsDock->setVisible(! d->toolSettingsDock->isVisible());
  }

  void MainWindow::toggleEngineSettingsDock()
  {
    ui.enginesDock->setVisible(! ui.enginesDock->isVisible() );
  }

} // end namespace Avogadro

#include "mainwindow.moc"
