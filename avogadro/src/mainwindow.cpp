/**********************************************************************
  MainWindow - main window, menus, main actions

  Copyright (C) 2006-2007 by Geoffrey R. Hutchison
  Copyright (C) 2006-2008 by Donald E. Curtis
  Copyright (C) 2007-2008 by Marcus D. Hanwell

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

#include "mainwindow.h"

#include <config.h>

#include "aboutdialog.h"
#include "addenginedialog.h"
#include "editcommands.h"
#include "settingsdialog.h"
#include "savedialog.h"

#include "enginelistview.h"
#include "engineprimitiveswidget.h"
#include "enginecolorswidget.h"

#include "icontabwidget.h"

//#ifdef Q_WS_MAC
//#include "macchempasteboard.h"
//#endif

#include <avogadro/pluginmanager.h>
#include <avogadro/camera.h>
#include <avogadro/extension.h>
#include <avogadro/primitive.h>
#include <avogadro/primitiveitemmodel.h>
#include <avogadro/toolgroup.h>
#include <avogadro/povpainter.h>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/forcefield.h>

#include <fstream>

#include <QClipboard>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QGLFramebufferObject>
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

#include <QDebug>

// This is a "hidden" exported Qt function on the Mac for Qt-4.x.
#ifdef Q_WS_MAC
 void qt_mac_set_menubar_icons(bool enable);
#endif

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  class MainWindowPrivate
  {
    public:
      MainWindowPrivate() : molecule( 0 ),
      undoStack( 0 ), toolsFlow( 0 ), toolsLayout( 0 ),
      toolsTab(0),
      toolSettingsStacked(0), toolSettingsWidget(0), toolSettingsDock(0),
      currentSelectedEngine(0),
      messagesText( 0 ),
      glWidget(0),
      centralLayout(0), centralTab(0), bottomFlat(0),
      toolGroup( 0 ),
      settingsDialog( 0 ), initialized( false )
    {}

      Molecule  *molecule;

      QString    fileName;
      QString    fileDialogPath;
      QUndoStack *undoStack;

      FlowLayout *toolsFlow;
      QVBoxLayout *toolsLayout;
      IconTabWidget *toolsTab;

      // we must manage this if we want it to be dynamic
      QStackedLayout *toolSettingsStacked;
      QWidget *toolSettingsWidget;
      QDockWidget *toolSettingsDock;

      QStackedLayout *enginesStacked;
      Engine         *currentSelectedEngine; // for settings widget title, etc.
//      QStackedLayout *engineConfigurationStacked;
//      QStackedLayout *enginePrimitivesStacked;

      QTextEdit *messagesText;

      QList<GLWidget *> glWidgets;
      GLWidget *glWidget;

      QVBoxLayout *centralLayout;
      QTabWidget *centralTab;
      FlatTabWidget *bottomFlat;

      ToolGroup *toolGroup;
      QAction    *actionRecentFile[MainWindow::maxRecentFiles];

      SettingsDialog *settingsDialog;

      // used for hideMainWindowMac() / showMainWindowMac()
      // save enable/disable status of every menu item
      QVector< QVector <bool> > menuItemStatus;
      bool initialized;

      bool tabbedTools;
  };

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

  void MainWindow::constructor()
  {

    ui.setupUi( this );
    // We cannot reliably set this via Designer
    // editing on Windows or Linux loses the flag
//    setUnifiedTitleAndToolBarOnMac(true);

    QSettings settings;
    pluginManager.readSettings(settings);

    d->tabbedTools = settings.value("tabbedTools", true).toBool();

    d->centralLayout = new QVBoxLayout(ui.centralWidget);

    // settings relies on the centralTab widget
    d->centralTab = new QTabWidget(ui.centralWidget);
    d->centralTab->setObjectName("centralTab");
    d->centralLayout->addWidget(d->centralTab);

    setAttribute( Qt::WA_DeleteOnClose );
    setAcceptDrops(true);

    // add our bottom flat tabs
    d->bottomFlat = new FlatTabWidget(this);
    d->centralLayout->addWidget(d->bottomFlat);

    d->undoStack = new QUndoStack( this );

    d->toolGroup = new ToolGroup( this );
    d->toolGroup->load();

    ui.menuDocks->addAction( ui.toolsDock->toggleViewAction() );

    d->enginesStacked = new QStackedLayout( ui.enginesWidget );
//    d->engineConfigurationStacked = new QStackedLayout( ui.engineConfigurationWidget );
//    d->enginePrimitivesStacked = new QStackedLayout( ui.enginePrimitivesWidget );

    // create messages widget
    QWidget *messagesWidget = new QWidget();
    QVBoxLayout *messagesVBox = new QVBoxLayout( messagesWidget );
    d->messagesText = new QTextEdit();
    d->messagesText->setReadOnly( true );
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

    ui.menuDocks->addAction( ui.projectDock->toggleViewAction() );
    ui.menuDocks->addAction( ui.enginesDock->toggleViewAction() );
//    ui.menuDocks->addAction( ui.engineConfigurationDock->toggleViewAction() );
//    ui.menuDocks->addAction( ui.enginePrimitivesDock->toggleViewAction() );
    ui.menuToolbars->addAction( ui.fileToolBar->toggleViewAction() );

    // Disable the "Revert" and "Save" actions -- we haven't modified anything
    // This will be enabled when the document is modified
    ui.actionRevert->setEnabled(false);
    ui.actionSave->setEnabled(false);

#ifdef Q_WS_MAC
    // Find the Avogadro global preferences action
    // and move it to the File menu (where it will be found)
    // for the Mac Application menu
    ui.menuSettings->removeAction( ui.configureAvogadroAction );
    ui.menuFile->addAction( ui.configureAvogadroAction );

    // Turn off the file toolbar (not really Mac-native)
    ui.fileToolBar->toggleViewAction();

    // Change the "Settings" menu to be Window
    ui.menuSettings->setTitle("Window");
    // and remove the trailing separator
    ui.menuSettings->removeAction( ui.menuSettings->actions().last() );

    // Remove all menu icons (violates Apple interface guidelines)
    // This is a not-quite-hidden Qt call on the Mac
    //    http://doc.trolltech.com/exportedfunctions.html
    qt_mac_set_menubar_icons(false);

    ui.menuSettings->setTitle("Window");
#endif

    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    connectUi();

    ui.projectDock->close();
  }

  bool MainWindow::event(QEvent *event)
  {
    // delayed initialization
    if(event->type() == QEvent::Polish)
    {
      reloadTabbedTools();

      loadExtensions();

      if(!molecule())
      {
        loadFile();
      }

      // read settings
      readSettings();

      // if we don't have a molecule then load a blank file
      d->initialized = true;
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

  bool MainWindow::tabbedTools() const
  {
    return d->tabbedTools;
  }

  void MainWindow::setTabbedTools(bool tabbedTools)
  {
    if(tabbedTools == d->tabbedTools)
    {
      return;
    }

    // set our new settings
    d->tabbedTools = tabbedTools;

    reloadTabbedTools();
  }

  bool MainWindow::renderAxes() const
  {
    return d->glWidget->renderAxes();
  }

  void MainWindow::setRenderAxes(bool render)
  {
    qDebug() << "setRenderAxes called: " << render;
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

  void MainWindow::reloadTabbedTools()
  {
    if(d->toolSettingsDock)
    {
      delete d->toolSettingsDock;
      d->toolSettingsDock = 0;
      d->toolSettingsWidget = 0;
      d->toolSettingsStacked = 0;
    }
    delete ui.toolsWidget;
    ui.toolsWidget = new QWidget();
    ui.toolsDock->setWidget(ui.toolsWidget);

    d->toolsTab = 0;
    d->toolsFlow = 0;
    d->toolsLayout = 0;

    if(d->tabbedTools)
    {
      d->toolsLayout = new QVBoxLayout(ui.toolsWidget);
      d->toolsLayout->setContentsMargins(0, 0, 0, 0);
      d->toolsLayout->setObjectName("toolsLayout");

      d->toolsTab = new IconTabWidget(ui.toolsWidget);
      d->toolsTab->setObjectName("toolsTab");
      d->toolsLayout->addWidget(d->toolsTab);

      // connect changing the tab
      connect(d->toolsTab, SIGNAL(currentChanged(int)),
          d->toolGroup, SLOT(setActiveTool(int)));
    }
    else
    {
      d->toolsFlow = new FlowLayout( ui.toolsWidget );
      d->toolsFlow->setMargin( 9 );

      d->toolSettingsDock = new QDockWidget(this);
      d->toolSettingsDock->setObjectName(QString::fromUtf8("toolSettingsDock"));
      d->toolSettingsDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::NoDockWidgetArea|Qt::RightDockWidgetArea);
      d->toolSettingsDock->setWindowTitle(tr("Tool Settings"));
      d->toolSettingsWidget = new QWidget(d->toolSettingsDock);
      d->toolSettingsWidget->setObjectName(QString::fromUtf8("toolSettingsWidget"));
      d->toolSettingsDock->setWidget(d->toolSettingsWidget);
      addDockWidget(static_cast<Qt::DockWidgetArea>(1), d->toolSettingsDock);

      d->toolSettingsStacked = new QStackedLayout( d->toolSettingsWidget );

      // add blank widget for those tools with no settings widget
      d->toolSettingsStacked->addWidget( new QWidget );
      ui.menuDocks->addAction( d->toolSettingsDock->toggleViewAction() );

      tabifyDockWidget(ui.toolsDock, d->toolSettingsDock);
      ui.toolsDock->raise();
    }

    const QList<Tool *> tools = d->toolGroup->tools();
    Tool *activeTool = d->toolGroup->activeTool();
    int toolCount = tools.size();

    for ( int i = 0; i < toolCount; i++ ) {
      Tool *tool = tools.at(i);
      connect(tool, SIGNAL(message(QString)), d->messagesText,
          SLOT(append(QString)));
      QAction *action = tool->activateAction();

      QWidget *widget = tools.at( i )->settingsWidget();

      if(d->tabbedTools)
      {
        if(!widget){
          widget = new QWidget();
        }

        QWidget *tmpWidget = new QWidget(d->toolsTab);
        QVBoxLayout *tmpLayout = new QVBoxLayout(tmpWidget);
        tmpLayout->setContentsMargins(0,0,0,0);

        // Add a "title" with the name of the tool
        QLabel *tmpLabel = new QLabel(tmpWidget);
        tmpLabel->setText(tool->name());
        tmpLabel->setAlignment(Qt::AlignHCenter);
        tmpLayout->addWidget(tmpLabel);

        tmpLayout->addWidget(widget);

        // add the tab
        int tabIndex = d->toolsTab->addTab(tmpWidget, action->icon(), QString());
        d->toolsTab->setTabToolTip(tabIndex, action->toolTip());

        // set the active tool
        if(tool == activeTool)
        {
          d->toolsTab->setCurrentIndex(tabIndex);
        }
      } // tabbed tools
      else
      { // non-tabbed tools
        // create a tool button
        QToolButton *button = new QToolButton( ui.toolsWidget );
        button->setDefaultAction( action );
        d->toolsFlow->addWidget( button );

        // if there is a settings widget then add it to the stack
        if(widget)
        {
          d->toolSettingsStacked->addWidget( widget );
          if ( i == 0 ) {
            d->toolSettingsStacked->setCurrentIndex( 1 );
          }
        }
      }
    }
//    d->toolGroup->setActiveTool(d->toolGroup->activeTool());
  }

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
    other->move( x() + 40, y() + 40 );
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
        + " (*.cml *.xyz *.ent *.pdb *.alc *.chm *.cdx *.cdxml *.c3d1 *.c3d2"
          " *.gpr *.mdl *.mol *.sdf *.sd *.crk3d *.cht *.dmol *.bgf"
          " *.gam *.inp *.gamin *.gamout *.tmol *.fract *.gau *.gzmat"
          " *.mpd *.mol2)"
        << tr("All files") + " (* *.*)"
        << tr("CML") + " (*.cml)"
        << tr("Crystallographic Interchange CIF") + " (*.cif)"
        << tr("GAMESS-US Output") + " (*.gamout)"
        << tr("Gaussian 98/03 Output") + " (*.g98 *.g03)"
        << tr("Gaussian Formatted Checkpoint") + " (*.fchk)"
        << tr("HyperChem") + " (*.hin)"
        << tr("MDL Mol") + " (*.mdl *.mol *.sd *.sdf)"
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
        qDebug() << "Loading File";
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
#ifdef Q_WS_MAC
        other->move( x() + 40, y() + 40 );
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

  bool MainWindow::loadFile( const QString &fileName )
  {
    if(fileName.isEmpty())
    {
      setFileName( fileName );
      setMolecule( new Molecule(this) );
      return true;
    }

    statusBar()->showMessage( tr("Loading %1...", "%1 is a filename").arg(fileName), 5000 );

    QFile file( fileName );
    if ( !file.open( QFile::ReadOnly | QFile::Text ) ) {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot read file %1:\n%2." )
          .arg( fileName )
          .arg( file.errorString() ) );
      return false;
    }
    file.close();

    QApplication::setOverrideCursor( Qt::WaitCursor );
    OBConversion conv;
    OBFormat     *inFormat = conv.FormatFromExt(( fileName.toAscii() ).data() );
    if ( !inFormat || !conv.SetInFormat( inFormat ) ) {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot read file format of file %1." )
          .arg( fileName ) );
      return false;
    }

    ifstream     ifs;
    ifs.open(( fileName.toAscii() ).data() );
    if ( !ifs ) { // shouldn't happen, already checked file above
      QApplication::restoreOverrideCursor();
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot read file %1." )
          .arg( fileName ) );
      return false;
    }

    statusBar()->showMessage( tr("Loading %1...").arg(fileName), 5000 );

    Molecule *molecule = new Molecule;
    if ( conv.Read( molecule, &ifs ) ) {
      if (molecule->GetDimension() != 3) {
	if (molecule->Has2D()) {
          int retval = QMessageBox::warning( this, tr( "Avogadro" ),
              tr( "This file contains 2D coordinates only. Do you want Avogadro "
              "to scale the bonds and do a quick optimization?"), QMessageBox::Yes, QMessageBox::No );

          if (retval == QMessageBox::Yes) {
	    // Scale the bond lengths  
            double sum = 0.0;
            FOR_BONDS_OF_MOL (bond, molecule) {
              sum += bond->GetLength();
            }
            double scale = (1.5 * molecule->NumBonds()) / sum;
            FOR_ATOMS_OF_MOL (atom, molecule) {
              vector3 vec = atom->GetVector();
              vec.SetX(vec.x() * scale); 
              vec.SetY(vec.y() * scale); 
              atom->SetVector(vec);
            }
            molecule->Center();

	    // place end atoms of wedge bonds at +1.0 Z
	    // place end atoms of hash bonds at -1.0 Z
            FOR_ATOMS_OF_MOL (atom, molecule) {
              FOR_BONDS_OF_ATOM (bond, &*atom) {
                if (bond->IsHash() && (&*atom == bond->GetBeginAtom())) {
                  vector3 vec = bond->GetEndAtom()->GetVector();
                  vec.SetZ(-1.0);
                  bond->GetEndAtom()->SetVector(vec);
                } else if (bond->IsWedge() && (&*atom == bond->GetBeginAtom())) {
                  vector3 vec = bond->GetEndAtom()->GetVector();
                  vec.SetZ(1.0);
                  bond->GetEndAtom()->SetVector(vec);
                }
              }
            }
            OBForceField *ff = OBForceField::FindForceField("UFF");
            if (ff) {
  	      ff->Setup(*molecule);
              ff->SteepestDescent(100);
              ff->GetCoordinates(*molecule);
	    }
	  }	  
	} else {
          QMessageBox::warning( this, tr( "Avogadro" ),
              tr( "This file does not contain 3D coordinates. You may not be able to edit or view properly." ));
	}
      }

      setMolecule( molecule );

      // do we have a multi-molecule file?
      // Changed this -- we have problems knowing if we're at the end of a gzipped file
      if (!fileName.endsWith(".gz", Qt::CaseInsensitive) &&
        ifs.peek() != EOF && ifs.good()) {
        QMessageBox::warning( this, tr( "Avogadro" ),
            tr( "This file appears to contain multiple molecule records."
              " Avogadro will only read the first molecule."
              " If you save, all other molecules may be lost." ));
      }

      QApplication::restoreOverrideCursor();

      QString status;
      QTextStream( &status ) << tr("Atoms: ") << d->molecule->NumAtoms() <<
        tr(" Bonds: ") << d->molecule->NumBonds();
      statusBar()->showMessage( status, 5000 );
    } else {
      QApplication::restoreOverrideCursor();
      statusBar()->showMessage( tr("Reading molecular file failed."), 5000 );
      return false;
    }

    setFileName( fileName );
    statusBar()->showMessage( tr("File Loaded..."), 5000 );
    d->toolGroup->setActiveTool(tr("Navigate"));
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
                        " *.mpd *.mol2)"
#endif
            << tr("CML") + " (*.cml)"
            << tr("GAMESS Input") + " (*.gamin)"
            << tr("Gaussian Cartesian Input") + " (*.gau)"
            << tr("Gaussian Z-matrix Input") + " (*.gzmat)"
            << tr("MDL SDfile") + "(*.mol)"
            << tr("PDB") + " (*.pdb)"
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

  bool MainWindow::saveFile( const QString &fileName )
  {
    // Check the format next (before we try creating a file)
    OBConversion conv;
    OBFormat     *outFormat = conv.FormatFromExt(( fileName.toAscii() ).data() );
    if ( !outFormat || !conv.SetOutFormat( outFormat ) ) {
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot write to file format of file %1." )
          .arg( fileName ) );
      return false;
    }

    QFile file( fileName );
    if ( !file.open( QFile::WriteOnly | QFile::Text ) ) {
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot write to the file %1:\n%2." )
          .arg( fileName )
          .arg( file.errorString() ) );
      return false;
    }
    file.close();
    
    // We'll save to a new file and then rename it to the requested file name
    // This way, if an error occurs, we won't destroy the old file
    QString newFileName(fileName);
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
    ofstream     ofs;
    ofs.open(( newFileName.toAscii() ).data() );
    if ( !ofs ) { // shouldn't happen, already checked file above
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot write to the file %1." )
          .arg( fileName ) );
      return false;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );
    statusBar()->showMessage( tr( "Saving file." ), 2000 );

    OBMol *molecule = dynamic_cast<OBMol*>( d->molecule );
    if ( conv.Write( molecule, &ofs ) ) {
      file.remove(); // remove the old file: WARNING -- would much prefer to just rename, but Qt won't let you
      newFile.rename(fileName);
      statusBar()->showMessage( tr("Save succeeded."), 5000 );
      setWindowModified( false );
    }
    else {
      statusBar()->showMessage( tr("Saving molecular file failed."), 5000 );
      newFile.remove(); // remove the temporary file -- we'll leave the old file in place
    }

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

    // Remove the filename ending - hopefully this is fairly robust
    QString file = d->fileName.mid(d->fileName.lastIndexOf("/")+1,
                           d->fileName.lastIndexOf("."));

    qDebug() << "Exported filename:" << file;

    QString fileName = SaveDialog::run(this,
                                       tr("Export Bitmap Graphics"),
                                       d->fileDialogPath,
                                       file,
                                       filters,
                                       "png",
                                       selectedFilter);

    settings.setValue("Export Graphics Filter", selectedFilter);

    if(fileName.isEmpty())
    {
      return;
    }

    qDebug() << "Exported filename:" << fileName;

    // render it (with alpha channel)
    QImage exportImage = d->glWidget->grabFrameBuffer( true );

    // now we embed molecular information into the file, if possible
    OBConversion conv;
    // MDL format is used for main copy -- atoms, bonds, chirality
    // supports either 2D or 3D, generic data
    OBFormat *mdlFormat = conv.FindFormat( "mdl" );
    QByteArray copyData;
    string output;
    if ( mdlFormat && conv.SetOutFormat( mdlFormat ) ) {
      output = conv.WriteString( d->molecule );
      copyData = output.c_str();
      // we embed the molfile into the image
      // e.g. http://baoilleach.blogspot.com/2007/08/access-embedded-molecular-information.html
      exportImage.setText("molfile", copyData);
    }

    // save a canonical SMILES too
    OBFormat *canFormat = conv.FindFormat( "can" );
    if ( canFormat && conv.SetOutFormat( canFormat ) ) {
      output = conv.WriteString( d->molecule );
      copyData = output.c_str();
      exportImage.setText("SMILES", copyData);
    }

    if ( !exportImage.save( fileName ) ) {
      QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "Cannot save file %1." ).arg( fileName ) );
      return;
    }
  }

  void MainWindow::exportPOV()
  {
    QSettings settings;
    QString selectedFilter = settings.value("Export POV-Ray Filter", tr("POV-Ray format") + " (*.pov)").toString();

    QStringList filters;
    filters << tr("POV-Ray format") + " (*.pov)"
            << tr("All files") + " (* *.*)";

    QString fileName = SaveDialog::run(this,
                                       tr("Export POV Scene"),
                                       d->fileDialogPath,
                                       d->fileName,
                                       filters,
                                       "pov",
                                       selectedFilter);

    settings.setValue("Export POV-Ray Filter", selectedFilter);

    if(fileName.isEmpty()) {
      return;
    }

    bool ok;
    int w = d->glWidget->width();
    int h = d->glWidget->height();
    double defaultAspectRatio = static_cast<double>(w)/h;
    double aspectRatio =
      QInputDialog::getDouble(0,
      QObject::tr("Set Aspect Ratio"),
      QObject::tr("The current Avogadro scene is %1x%2 pixels large, "
          "and therefore has aspect ratio %3.\n"
          "You may keep this value, for example if you intend to use POV-Ray\n"
          "to produce an image of %4x1000 pixels, "
          "or you may enter any other positive value,\n"
          "for example 1 if you intend to use POV-Ray to produce a square image, "
          "like 1000x1000 pixels.")
          .arg(w).arg(h).arg(defaultAspectRatio)
          .arg(static_cast<int>(1000*defaultAspectRatio)),
      defaultAspectRatio,
      0.1,
      10,
      6,
      &ok);

    if(ok)
      POVPainterDevice pd( fileName, aspectRatio, d->glWidget );
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

  bool MainWindow::maybeSave()
  {
    if ( isWindowModified() ) {
      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning( this, tr( "Avogadro" ),
          tr( "The document has been modified.\n"
            "Do you want to save your changes?" ),
          QMessageBox::Save | QMessageBox::Discard
          | QMessageBox::Cancel );

      if ( ret == QMessageBox::Save )
        return save();
      else if ( ret == QMessageBox::Cancel )
        return false;
    }
    return true;
  }

  void MainWindow::clearRecentFiles()
  {
    QSettings settings; // already set up properly via main.cpp
    QStringList files;
    settings.setValue( "recentFileList", files );

    updateRecentFileActions();
  }

  void MainWindow::about()
  {
    AboutDialog * about = new AboutDialog( this );
    about->show();
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
//    d->engineConfigurationStacked->setCurrentIndex( index );
//    d->enginePrimitivesStacked->setCurrentIndex( index );
    ui.actionDisplayAxes->setChecked(renderAxes());
    ui.actionDebugInformation->setChecked(renderDebug());
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
    Molecule newMol;

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

      newMol.Translate( offset );
      PasteCommand *command = new PasteCommand( d->molecule, newMol, d->glWidget );
      d->undoStack->push( command );
      d->toolGroup->setActiveTool(tr("Manipulate")); // set the tool to manipulate, so we can immediate move the selection
    } else {
      return false;
    }
    return true;
  }

  // Helper function -- works for "cut" or "copy"
  // FIXME add parameter to set "Copy" or "Cut" in messages
  QMimeData* MainWindow::prepareClipboardData( PrimitiveList selectedItems )
  {
    QMimeData *mimeData = new QMimeData;
    // we also save an image for copy/paste to office programs, presentations, etc.
    QImage clipboardImage = d->glWidget->grabFrameBuffer( true );

    Molecule *moleculeCopy = d->molecule;
    if ( !selectedItems.isEmpty() ) { // we only want to copy the selected items
      moleculeCopy = new Molecule;
      std::map<OBAtom*, OBAtom*> AtomMap; // key is from old, value from new
      // copy atoms and create a map of atom indexes
      foreach( Primitive *item, selectedItems.subList(Primitive::AtomType) ) {
        OBAtom *selected = static_cast<Atom*>( item );
        moleculeCopy->InsertAtom( *selected );
        AtomMap[selected] = moleculeCopy->GetAtom( moleculeCopy->NumAtoms() );
      }

      // use the atom map to map bonds
      map<OBAtom*, OBAtom*>::iterator posBegin, posEnd;
      FOR_BONDS_OF_MOL( b, d->molecule ) {
        posBegin = AtomMap.find( b->GetBeginAtom() );
        posEnd = AtomMap.find( b->GetEndAtom() );
        // make sure both bonds are in the map (i.e. selected)
        if ( posBegin != AtomMap.end() && posEnd != AtomMap.end() ) {
          moleculeCopy->AddBond(( posBegin->second )->GetIdx(),
              ( posEnd->second )->GetIdx(),
              b->GetBO(), b->GetFlags() );
        }
      } // end looping over bonds
    } // should now have a copy of our selected fragment

    OBConversion conv;
    // MDL format is used for main copy -- atoms, bonds, chirality
    // supports either 2D or 3D, generic data
    // CML is another option, but not as well tested in Open Babel
    OBFormat *mdlFormat = conv.FindFormat( "mdl" );
    if ( !mdlFormat || !conv.SetOutFormat( mdlFormat ) ) {
      statusBar()->showMessage( tr( "Copy failed (mdl unavailable)." ), 5000 );
      return NULL; // nothing in it yet
    }

    // write an MDL file first (with bond orders, radicals, etc.)
    // (CML might be better in the future, but this works well now)
    string output = conv.WriteString( moleculeCopy );
    QByteArray copyData( output.c_str(), output.length() );
    mimeData->setData( "chemical/x-mdl-molfile", copyData );

    // we embed the molfile into the image
    // e.g. http://baoilleach.blogspot.com/2007/08/access-embedded-molecular-information.html
    clipboardImage.setText("molfile", copyData);

    // save a canonical SMILES too
    OBFormat *canFormat = conv.FindFormat( "can" );
    if ( canFormat && conv.SetOutFormat( canFormat ) ) {
      output = conv.WriteString( moleculeCopy );
      copyData = output.c_str();
      clipboardImage.setText("SMILES", copyData);
    }

    // Copy XYZ coordinates to the text selection buffer
    OBFormat *xyzFormat = conv.FindFormat( "xyz" );
    if ( xyzFormat && conv.SetOutFormat( xyzFormat ) ) {
      output = conv.WriteString( moleculeCopy );
      copyData = output.c_str();
      mimeData->setText( QString( copyData ) );
    }

    // need to free our temporary moleculeCopy
    if ( !selectedItems.isEmpty() ) {
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
    FOR_ATOMS_OF_MOL( a, d->molecule ) {
      Atom *atom = static_cast<Atom*>( &*a );
      selection.append( atom );
    }
    FOR_BONDS_OF_MOL( b, d->molecule ) {
      Bond *bond = static_cast<Bond*>( &*b );
      selection.append( bond );
    }
    d->glWidget->setSelected( selection, true );

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

  int MainWindow::painterQuality()
  {
    return d->glWidget->quality();
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

    QString tabName = tr("View %1").arg( QString::number( d->centralTab->count()+1) );

    d->centralTab->addTab(widget, tabName);
    ui.actionDisplayAxes->setChecked(gl->renderAxes());
    ui.actionDebugInformation->setChecked(gl->renderDebug());
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

    QString tabName = tr("View %1").arg( QString::number( d->centralTab->count()+1) );

    d->centralTab->addTab( widget, tabName );
    ui.actionCloseView->setEnabled( true );
    ui.actionDisplayAxes->setChecked(gl->renderAxes());
    ui.actionDebugInformation->setChecked(gl->renderDebug());

    writeSettings();
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

        // delete the engine configuration for this GLWidget
//        widget = d->engineConfigurationStacked->widget( index );
//        d->engineConfigurationStacked->removeWidget( widget );
//        delete widget;

        // delete the engine primitives for this GLWidget
//        widget = d->enginePrimitivesStacked->widget( index );
//        d->enginePrimitivesStacked->removeWidget( widget );
//        delete widget;

        for ( int count=d->centralTab->count(); index < count; index++ ) {
          d->centralTab->setTabText( index, tr( "View %1" ).arg( QString::number( index + 1 ) ) );
        }
        d->glWidgets.removeAll( glWidget );
        delete glWidget;
        ui.actionCloseView->setEnabled( d->centralTab->count() != 1 );
      }
    }

    setView( d->centralTab->currentIndex() );

    writeSettings();
  }

  void MainWindow::centerView()
  {
    d->glWidget->camera()->initializeViewPoint();
    d->glWidget->update();
  }

  void MainWindow::fullScreen()
  {
    if ( !this->isFullScreen() ) {
      ui.actionFullScreen->setText( tr( "Normal Size" ) );
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
      ui.fileToolBar->show();
      statusBar()->show();
    }
  }

  void MainWindow::showSettingsDialog()
  {
    if ( !d->settingsDialog ) {
      d->settingsDialog = new SettingsDialog( this );
    }
    d->settingsDialog->show();
  }

  void MainWindow::setBackgroundColor()
  {
    d->glWidget->setBackground( QColorDialog::getColor( d->glWidget->background(), this ) );
    d->glWidget->update();
  }

  void MainWindow::setTool( Tool *tool )
  {
    if(d->tabbedTools && d->toolsTab)
    {
      int index = d->toolGroup->tools().indexOf(tool);
      d->toolsTab->setCurrentIndex(index);
    }
    else if(d->toolSettingsStacked)
    {
      if ( tool->settingsWidget() ) {
        d->toolSettingsStacked->setCurrentWidget( tool->settingsWidget() );
      } else {
        d->toolSettingsStacked->setCurrentIndex( 0 );
      }
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
    connect( ui.actionClose, SIGNAL( triggered() ), this, SLOT( closeFile() ) );
    connect( ui.actionCloseTool, SIGNAL( triggered() ), this, SLOT( closeFile() ) );
#endif
    connect( ui.actionSave, SIGNAL( triggered() ), this, SLOT( save() ) );
    connect( ui.actionSaveTool, SIGNAL( triggered() ), this, SLOT( save() ) );
    connect( ui.actionSaveAs, SIGNAL( triggered() ), this, SLOT( saveAs() ) );
    connect( ui.actionRevert, SIGNAL( triggered() ), this, SLOT( revert() ) );
    connect( ui.actionExportGraphics, SIGNAL( triggered() ), this, SLOT( exportGraphics() ) );
    ui.actionExportGraphics->setEnabled( QGLFramebufferObject::hasOpenGLFramebufferObjects() );
    connect( ui.actionExportPOV, SIGNAL( triggered() ), this, SLOT( exportPOV() ) );
#ifdef Q_WS_MAC
    connect( ui.actionQuit, SIGNAL( triggered() ), this, SLOT( macQuit() ) );
    connect( ui.actionQuitTool, SIGNAL( triggered() ), this, SLOT( macQuit() ) );
#else
    connect( ui.actionQuit, SIGNAL( triggered() ), this, SLOT( close() ) );
    connect( ui.actionQuitTool, SIGNAL( triggered() ), this, SLOT( close() ) );
#endif

    connect( ui.actionClearRecent, SIGNAL( triggered() ), this, SLOT( clearRecentFiles() ) );

    connect( d->undoStack, SIGNAL( cleanChanged( bool ) ), this, SLOT( undoStackClean( bool ) ) );

    connect( ui.actionCut, SIGNAL( triggered() ), this, SLOT( cut() ) );
    connect( ui.actionCopy, SIGNAL( triggered() ), this, SLOT( copy() ) );
    connect( ui.actionPaste, SIGNAL( triggered() ), this, SLOT( paste() ) );
    connect( ui.actionClear, SIGNAL( triggered() ), this, SLOT( clear() ) );
    connect( ui.actionSelect_All, SIGNAL( triggered() ), this, SLOT( selectAll() ) );
    connect( ui.actionSelect_None, SIGNAL( triggered() ), this, SLOT( selectNone() ) );

    connect( ui.actionNewView, SIGNAL( triggered() ), this, SLOT( newView() ) );
    connect( ui.actionDuplicateView, SIGNAL( triggered() ), this, SLOT( duplicateView() ) );
    connect( ui.actionCloseView, SIGNAL( triggered() ), this, SLOT( closeView() ) );
    connect( ui.actionCenter, SIGNAL( triggered() ), this, SLOT( centerView() ) );
    connect( ui.actionFullScreen, SIGNAL( triggered() ), this, SLOT( fullScreen() ) );
    connect( ui.actionSetBackgroundColor, SIGNAL( triggered() ), this, SLOT( setBackgroundColor() ) );
    connect(ui.actionDisplayAxes, SIGNAL(triggered(bool)),
            this, SLOT(setRenderAxes(bool)));
    connect(ui.actionDebugInformation, SIGNAL(triggered(bool)),
            this, SLOT(setRenderDebug(bool)));
    connect( ui.actionAbout, SIGNAL( triggered() ), this, SLOT( about() ) );

    connect( d->centralTab, SIGNAL( currentChanged( int ) ), this, SLOT( setView( int ) ) );

    connect( ui.configureAvogadroAction, SIGNAL( triggered() ),
        this, SLOT( showSettingsDialog() ) );

    connect( ui.pluginManagerAction, SIGNAL( triggered() ),
        &pluginManager, SLOT( showDialog() ) );

    connect( ui.actionTutorials, SIGNAL( triggered() ), this, SLOT( openTutorialURL() ));
    connect( ui.actionFAQ, SIGNAL( triggered() ), this, SLOT( openFAQURL() ) );
    connect( ui.actionRelease_Notes, SIGNAL( triggered() ), this, SLOT( openReleaseNotesURL() ));
    connect( ui.actionAvogadro_Website, SIGNAL( triggered() ), this, SLOT( openWebsiteURL() ) );
    connect( ui.actionReport_a_Bug, SIGNAL( triggered() ), this, SLOT( openBugURL() ) );
    

    connect( d->toolGroup, SIGNAL( toolActivated( Tool * ) ), this, SLOT( setTool( Tool * ) ) );
    connect( this, SIGNAL( moleculeChanged( Molecule * ) ), d->toolGroup, SLOT( setMolecule( Molecule * ) ) );

  }

  void MainWindow::setMolecule( Molecule *molecule )
  {
    if ( d->molecule ) {
      disconnect( d->molecule, 0, this, 0 );
      d->molecule->deleteLater();
    }

    d->undoStack->clear();

    d->molecule = molecule;
    connect( d->molecule, SIGNAL( primitiveAdded( Primitive * ) ), this, SLOT( documentWasModified() ) );
    connect( d->molecule, SIGNAL( primitiveUpdated( Primitive * ) ), this, SLOT( documentWasModified() ) );
    connect( d->molecule, SIGNAL( primitiveRemoved( Primitive * ) ), this, SLOT( documentWasModified() ) );

    emit moleculeChanged(molecule);

    ui.projectTree->setModel( new PrimitiveItemModel( d->molecule, this ) );

    setWindowModified( false );
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
      setWindowTitle( tr( "%1[*] - %2" ).arg( fileInfo.fileName() )
          .arg( tr( "Avogadro" ) ) );

      QSettings settings; // already set up properly via main.cpp
      QStringList files = settings.value( "recentFileList" ).toStringList();
      files.removeAll( fileName );
      files.prepend( fileName );
      while ( files.size() > maxRecentFiles )
        files.removeLast();

      settings.setValue( "recentFileList", files );
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
    // On Mac or Windows, the application should remember
    // window positions. On Linux, it's handled by the window manager
#if defined (Q_WS_MAC) || defined (Q_WS_WIN)
    QPoint originalPosition = pos();
    QPoint newPosition = settings.value("pos", QPoint(200, 200)).toPoint();

    // We'll try moving the window. If it moves off-screen, we'll move it back
    // This solves PR#1903437
    move(newPosition);
    QDesktopWidget desktop;
    if (desktop.screenNumber(this) == -1) // it's not on a screen
      move(originalPosition);
#endif
    QSize size = settings.value( "size", QSize( 640, 480 ) ).toSize();
    resize( size );

    QByteArray ba = settings.value( "state" ).toByteArray();
    if(!ba.isEmpty())
    {
      restoreState(ba);
    } else {
//      tabifyDockWidget(ui.enginesDock, ui.engineConfigurationDock);
//      tabifyDockWidget(ui.enginesDock, ui.enginePrimitivesDock);
      ui.enginesDock->raise();
    }

    setTabbedTools(settings.value( "tabbedTools", true ).toBool());

    settings.beginGroup("tools");
    d->toolGroup->readSettings(settings);
    settings.endGroup();

    int count = settings.beginReadArray("view");
    if(count < 1)
    {
      count = 1;
    }
    for(int i = 0; i<count; i++)
    {
      settings.setArrayIndex(i);
      QWidget *widget = new QWidget();
      QVBoxLayout *layout = new QVBoxLayout( widget );
      layout->setMargin( 0 );
      layout->setSpacing( 6 );
      GLWidget *gl = newGLWidget();
      layout->addWidget(gl);

      QString tabName = tr("View %1").arg(QString::number(i+1));
      d->centralTab->addTab(widget, tabName);

      gl->readSettings(settings);
    }
    settings.endArray();

    // Set the view conditions for the initial view
    ui.actionDisplayAxes->setChecked(renderAxes());
    ui.actionDebugInformation->setChecked(renderDebug());

    ui.actionCloseView->setEnabled(count > 1);
  }

  void MainWindow::writeSettings()
  {
    QSettings settings;
#if defined (Q_WS_MAC) || defined (Q_WS_WIN)
    settings.setValue( "pos", pos() );
#endif
    settings.setValue( "size", size() );
    settings.setValue( "state", saveState() );

    settings.setValue( "tabbedTools", d->tabbedTools );
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
    
    // write the plugin manager settings
    settings.beginGroup("plugins");
    pluginManager.writeSettings(settings);
    settings.endGroup();
  }

  void MainWindow::loadExtensions()
  {
    QString prefixPath = QString(INSTALL_PREFIX) + '/'
      + QString(INSTALL_LIBDIR) + "/avogadro/extensions";
    QStringList pluginPaths;
    pluginPaths << prefixPath;

#ifdef WIN32
	pluginPaths << QCoreApplication::applicationDirPath() + "/extensions";
#endif

    // Krazy: Use QProcess:
    // http://doc.trolltech.com/4.3/qprocess.html#systemEnvironment
    if (getenv("AVOGADRO_EXTENSIONS") != NULL) {
      pluginPaths = QString(getenv("AVOGADRO_EXTENSIONS") ).split(':');
    }

    foreach(const QString& path, pluginPaths)
    {
      QDir dir(path);
      //      qDebug() << "SearchPath:" << dir.absolutePath() << endl;
      foreach(const QString& fileName, dir.entryList(QDir::Files))
      {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject *instance = loader.instance();
        // qDebug() << "File: " << fileName;
        ExtensionFactory *factory = qobject_cast<ExtensionFactory *>(instance);
        if ( factory )
        {
          Extension *extension = factory->createInstance(this);
          qDebug() << "Found Extension: " << extension->name() << " - "
                   << extension->description();

          QList<QAction *>actions = extension->actions();

          foreach(QAction *action, actions)
          {
            // Here's the fun part, we go customize our menus
            // Add these actions to the menu described by the menuPath
            QString menuPathString = extension->menuPath(action);
            QMenu *path = NULL;

            if ( menuPathString.size() ) {
              QStringList menuPath = menuPathString.split( ">" );
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

            if(!path)
            {
              path = ui.menuExtensions;
            }

            path->addAction( action );
            connect( action, SIGNAL( triggered() ), this, SLOT( actionTriggered() ) );
          }

          QDockWidget *dockWidget = extension->dockWidget();
          if(dockWidget)
          {
            addDockWidget(Qt::RightDockWidgetArea, dockWidget);
            ui.menuDocks->addAction(dockWidget->toggleViewAction());
          }

          connect(this, SIGNAL( moleculeChanged(Molecule*)),
                  extension, SLOT(setMolecule(Molecule*)));
          // When loading a molecule with an already open window with another
          // molecule this signal is never triggered. If we already have a
          // molecule at the point of loading then set it for the extension
          if (d->molecule)
            extension->setMolecule(d->molecule);
          connect(extension, SIGNAL( message(QString)),
                  d->messagesText, SLOT(append(QString)));
        }
      }
    }
  }

  void MainWindow::actionTriggered()
  {
    QAction *action = qobject_cast<QAction *>( sender() );
    if ( action ) {
      Extension *extension = dynamic_cast<Extension *>( action->parent() );

      QUndoCommand *command = 0;
      command = extension->performAction( action, d->glWidget);

      if ( command ) {
        d->undoStack->push( command );
      }
    }
  }

  void MainWindow::hideMainWindowMac()
  {
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
  }

  void MainWindow::showMainWindowMac()
  {
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

    connect( this, SIGNAL( moleculeChanged( Molecule * ) ), gl, SLOT( setMolecule( Molecule * ) ) );
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
    int nEngines = pluginManager.engineFactories().size() - 1;
    int nTools = d->glWidget->toolGroup()->tools().size();
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

    QWidget *settingsWindow = new QWidget();
    settingsWindow->setWindowTitle(d->currentSelectedEngine->name() + ' ' + tr("Settings"));
    QVBoxLayout *layout = new QVBoxLayout;

    // now set up the tabs: Currently settings and objects (primitives)
    QTabWidget *settingsTabs = new QTabWidget(settingsWindow);
    settingsTabs->addTab(d->currentSelectedEngine->settingsWidget(), tr("Settings"));
    
    EnginePrimitivesWidget *primitivesWidget =
          new EnginePrimitivesWidget(d->glWidget, settingsWindow);
    primitivesWidget->setEngine(d->currentSelectedEngine);
    settingsTabs->addTab(primitivesWidget, tr("Objects"));
    
    EngineColorsWidget *colorsWidget = new EngineColorsWidget(settingsWindow);
    colorsWidget->setEngine(d->currentSelectedEngine);
    settingsTabs->addTab(colorsWidget, tr("Colors"));

    layout->addWidget(settingsTabs);
    settingsWindow->setLayout(layout);
    settingsWindow->show();
  }

  void MainWindow::addEngineClicked()
  {
    Engine *engine =  AddEngineDialog::getEngine(this, pluginManager.engineFactories());
    if(engine) {
      PrimitiveList p = d->glWidget->selectedPrimitives();
      if(!p.size()) {
        p = d->glWidget->primitives();
      }
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
            newEngine->setPrimitives(d->glWidget->selectedPrimitives());
          } else {
            newEngine->setPrimitives(d->glWidget->primitives());
          }
          newEngine->setName(newEngine->name() + tr(" copy"));
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

} // end namespace Avogadro

#include "mainwindow.moc"
