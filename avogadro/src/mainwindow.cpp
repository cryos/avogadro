/**********************************************************************
  MainWindow - main window, menus, main actions

  Copyright (C) 2006-2007 by Geoffrey R. Hutchison
  Copyright (C) 2006-2008 by Donald E. Curtis
  Copyright (C) 2007-2009 by Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

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

#include <avogadro/dockextension.h>
#include <avogadro/dockwidget.h>

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
#include "engineviewwidget.h"
#include "engineprimitiveswidget.h"
#include "primitiveitemmodel.h"
#include "enginecolorswidget.h"

#include "glgraphicsview.h"
#include "detachedview.h"

#ifdef ENABLE_UPDATE_CHECKER
  #include "updatecheck.h"
#endif

//#ifdef Q_WS_MAC
//#include "macchempasteboard.h"
//#endif

#include <avogadro/pluginmanager.h>

// Does not work for me with out of source builds at least - ui_projecttreeeditor.h
// can't be found and avogadro should be able to build without libavogadro
#include "projecttreeeditor.h"
#include "projecttreemodel.h"

#include <avogadro/camera.h>
#include <avogadro/extension.h>
#include <avogadro/engine.h>

#include <avogadro/moleculefile.h>

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
#include <QProcess>
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
#include <QTableWidget>
#include <QProgressDialog>

#include <QDebug>

#include <Eigen/Geometry>
#define USEQUAT
// This is a "hidden" exported Qt function on the Mac for Qt-4.x.
#ifdef Q_WS_MAC
 void qt_mac_set_menubar_icons(bool enable);
#endif

#ifdef QTTESTING
#include <pqTestUtility.h>
#include <pqEventObserver.h>
#include <pqEventSource.h>
#include <QXmlStreamReader>
#endif

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro
{
#ifdef QTTESTING
class XMLEventObserver : public pqEventObserver
{
  QXmlStreamWriter* XMLStream;
  QString XMLString;

public:
  XMLEventObserver(QObject* p) : pqEventObserver(p)
  {
    this->XMLStream = NULL;
  }
  ~XMLEventObserver()
  {
    delete this->XMLStream;
  }

protected:
  virtual void setStream(QTextStream* stream)
  {
    if (this->XMLStream) {
      this->XMLStream->writeEndElement();
      this->XMLStream->writeEndDocument();
      delete this->XMLStream;
      this->XMLStream = NULL;
    }
    if (this->Stream)
      *this->Stream << this->XMLString;

    this->XMLString = QString();
    pqEventObserver::setStream(stream);
    if (this->Stream) {
      this->XMLStream = new QXmlStreamWriter(&this->XMLString);
      this->XMLStream->setAutoFormatting(true);
      this->XMLStream->writeStartDocument();
      this->XMLStream->writeStartElement("events");
    }
  }

  virtual void onRecordEvent(const QString& widget, const QString& command,
                             const QString& arguments)
  {
    if(this->XMLStream) {
      this->XMLStream->writeStartElement("event");
      this->XMLStream->writeAttribute("widget", widget);
      this->XMLStream->writeAttribute("command", command);
      this->XMLStream->writeAttribute("arguments", arguments);
      this->XMLStream->writeEndElement();
    }
  }
};

class XMLEventSource : public pqEventSource
{
  typedef pqEventSource Superclass;
  QXmlStreamReader *XMLStream;

public:
  XMLEventSource(QObject* p): Superclass(p) { this->XMLStream = NULL;}
  ~XMLEventSource() { delete this->XMLStream; }

protected:
  virtual void setContent(const QString& xmlfilename)
  {
    delete this->XMLStream;
    this->XMLStream = NULL;

    QFile xml(xmlfilename);
    if (!xml.open(QIODevice::ReadOnly)) {
      qDebug() << "Failed to load " << xmlfilename;
      return;
    }
    QByteArray data = xml.readAll();
    this->XMLStream = new QXmlStreamReader(data);
  }

  int getNextEvent(QString& widget, QString& command, QString& arguments)
  {
    if (this->XMLStream->atEnd())
      return DONE;
    while (!this->XMLStream->atEnd()) {
      QXmlStreamReader::TokenType token = this->XMLStream->readNext();
      if (token == QXmlStreamReader::StartElement) {
        if (this->XMLStream->name() == "event")
          break;
      }
    }
    if (this->XMLStream->atEnd())
      return DONE;

    widget = this->XMLStream->attributes().value("widget").toString();
    command = this->XMLStream->attributes().value("command").toString();
    arguments = this->XMLStream->attributes().value("arguments").toString();
    return SUCCESS;
  }
};
#endif

  enum BuilderOption {
    AskUser = 0,
    AlwaysBuild = 1,
    NeverBuild = 2
  };

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
      build3D(AskUser),
      moleculeFile(0), currentIndex(0),
      progressDialog(0),
      allMoleculesTable(0),
      allMoleculesDialog(0)
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

    // Pointer to an action group for "View > Projection"
    QActionGroup *projectionGroup;

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

    BuilderOption build3D;
    // Track all the molecules in a file
    MoleculeFile *moleculeFile;
    unsigned int currentIndex;
    QProgressDialog *progressDialog;
    QTableWidget  *allMoleculesTable;
    QDialog       *allMoleculesDialog;

    QMap<Engine*, QWidget*> engineSettingsWindows;
  };

  const int MainWindow::m_configFileVersion = 3;

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
    d->centralTab->setDocumentMode(true);
    d->centralTab->setTabsClosable(false); // by default, we just have one tab
    d->centralLayout->addWidget(d->centralTab);
    // Set up the signal/slot for closing tabs from the tab widget
    connect(d->centralTab, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeView(int)));

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
    QAction *redoAction = d->undoStack->createRedoAction( this );
    #if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
      // Load icons from desktop theme
      // File
      const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/icons/document-new.png"));
      const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/icons/document-open.png"));
      const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/icons/document-save.png"));
      const QIcon saveAsIcon = QIcon::fromTheme("document-save-as", QIcon(":/icons/document-save-as.png"));
      const QIcon revertIcon = QIcon::fromTheme("document-revert", QIcon(":/icons/document-revert.png"));
      const QIcon closeIcon = QIcon::fromTheme("document-close", QIcon(":/icons/document-close.png"));
      const QIcon importIcon = QIcon::fromTheme("document-import", QIcon(":/icons/document-import.png"));
      const QIcon exportIcon = QIcon::fromTheme("document-export", QIcon(":/icons/document-export.png"));
      const QIcon quitIcon = QIcon::fromTheme("application-exit", QIcon(":/icons/application-exit.png"));
      // Edit
      const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(":/icons/edit-undo.png"));
      const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(":/icons/edit-redo.png"));
      const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(":/icons/edit-cut.png"));
      const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/icons/edit-copy.png"));
      const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/icons/edit-paste.png"));
      const QIcon clearIcon = QIcon::fromTheme("edit-clear", QIcon(":/icons/edit-clear.png"));
    #else
      // Load icons
      // File
      const QIcon newIcon = QIcon(":/icons/document-new.png");
      const QIcon openIcon = QIcon(":/icons/document-open.png");
      const QIcon saveIcon = QIcon(":/icons/document-save.png");
      const QIcon saveAsIcon = QIcon(":/icons/document-save-as.png");
      const QIcon revertIcon = QIcon(":/icons/document-revert.png");
      const QIcon closeIcon = QIcon(":/icons/document-close.png");
      const QIcon importIcon = QIcon(":/icons/document-import.png");
      const QIcon exportIcon = QIcon(":/icons/document-export.png");
      const QIcon quitIcon = QIcon(":/icons/application-exit.png");
      // Edit
      const QIcon undoIcon = QIcon(":/icons/edit-undo.png");
      const QIcon redoIcon = QIcon(":/icons/edit-redo.png");
      const QIcon cutIcon = QIcon(":/icons/edit-cut.png");
      const QIcon copyIcon = QIcon(":/icons/edit-copy.png");
      const QIcon pasteIcon = QIcon(":/icons/edit-paste.png");
      const QIcon clearIcon = QIcon(":/icons/edit-clear.png");
    #endif
    // Set toolbar icons
    ui.actionNewTool->setIcon(newIcon);
    ui.actionOpenTool->setIcon(openIcon);
    ui.actionSaveTool->setIcon(saveIcon);
    ui.actionCloseTool->setIcon(closeIcon);
    ui.actionQuitTool->setIcon(quitIcon);
    // Set file menu icons
    ui.actionNew->setIcon(newIcon);
    ui.actionOpen->setIcon(openIcon);
    ui.actionClose->setIcon(closeIcon);
    ui.actionSave->setIcon(saveIcon);
    ui.actionSaveAs->setIcon(saveAsIcon);
    ui.actionRevert->setIcon(revertIcon);
    ui.actionImport_File->setIcon(importIcon);
    ui.actionExportGraphics->setIcon(exportIcon);
    ui.actionExportGL2PS->setIcon(exportIcon);
    // Set edit menu icons
    undoAction->setIcon(undoIcon);
    redoAction->setIcon(redoIcon);
    ui.actionCut->setIcon(cutIcon);
    ui.actionCopy->setIcon(copyIcon);
    ui.actionPaste->setIcon(pasteIcon);
    ui.actionClear->setIcon(clearIcon);

    undoAction->setShortcuts( QKeySequence::Undo );
    redoAction->setShortcuts( QKeySequence::Redo );
    ui.actionClear->setShortcuts( QList<QKeySequence>() << QKeySequence("Backspace") << QKeySequence("Del"));
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

    // Create and assign an action group for "View > Projection"
    d->projectionGroup = new QActionGroup(this);
    d->projectionGroup->addAction(ui.actionPerspective);
    d->projectionGroup->addAction(ui.actionOrthographic);

#ifdef Q_WS_MAC
    // Find the Avogadro global preferences action
    // and make sure it ends up in the Mac Application menu
    ui.configureAvogadroAction->setMenuRole(QAction::PreferencesRole);

    // Remove the last separator in the File menu
    QList <QAction *> actions = ui.menuFile->actions();
    QAction *lastAction;
    if (actions.last()->isSeparator()) {
      lastAction = actions.last();
      ui.menuFile->removeAction(lastAction);
    }
    else if (actions[actions.size() - 2]->isSeparator()) { // "Quit" menu item hasn't moved yet
      lastAction = actions[actions.size() - 2];
      ui.menuFile->removeAction(lastAction);
    }

    // Remove the first separator in the help menu (this remains even though the "About" item moves).
    QAction *firstAction = ui.menuHelp->actions().first();
    if (firstAction->isSeparator())
      ui.menuHelp->removeAction(firstAction);
    else { // the "About" menu item hasn't moved yet.
      firstAction = ui.menuHelp->actions()[1];
      if (firstAction->isSeparator())
        ui.menuHelp->removeAction(firstAction);
    }

    // Turn off the file toolbar (not really Mac-native)
    // Fixes PR#1965004
    ui.menuToolbars->removeAction( ui.fileToolBar->toggleViewAction() );
    ui.fileToolBar->hide();

    // Change the "Settings" menu to be Window
    ui.menuSettings->setTitle(tr("Window"));
    firstAction = ui.menuSettings->actions().first();
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

    //    ui.menuSettings->addSeparator();
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
    m_updateCheck = UpdateCheck::getInstance(this);
#endif

    m_ignoreConfig = false;

#ifdef QTTESTING
    QAction *actionRecord = new QAction(this);
    actionRecord->setText(tr("Record Test..."));
    ui.menuSettings->addAction(actionRecord);
    QAction *actionPlay = new QAction(this);
    actionPlay->setText(tr("Play Test..."));
    ui.menuSettings->addAction(actionPlay);

    connect(actionRecord, SIGNAL(triggered()), this, SLOT(record()));
    connect(actionPlay, SIGNAL(triggered()), this, SLOT(play()));
    this->TestUtility = new pqTestUtility(this);
    this->TestUtility->addEventObserver("xml", new XMLEventObserver(this));
    this->TestUtility->addEventSource("xml", new XMLEventSource(this));
#endif

    static const QStringList searchDirs = pluginSearchDirs();
    d->pluginManager.setPluginPath(searchDirs);
  }

#ifdef QTTESTING
  void MainWindow::record()
  {
    QString filename = QFileDialog::getSaveFileName (this, "Test File Name",
      QString(), "XML Files (*.xml)");
    if (!filename.isEmpty())
      {
      this->TestUtility->recordTests(filename);
      }
  }

  void MainWindow::play()
  {
    QString filename = QFileDialog::getOpenFileName (this, "Test File Name",
      QString(), "XML Files (*.xml)");
    if (!filename.isEmpty())
      {
      this->TestUtility->playTests(filename);
      }
  }

  void MainWindow::popup()
  {
    QDialog dialog;
    QHBoxLayout* hbox = new QHBoxLayout(&dialog);
    QPushButton button("Click to Close", &dialog);
    hbox->addWidget(&button);
    QObject::connect(&button, SIGNAL(clicked()), &dialog, SLOT(accept()));
    dialog.exec();
  }
#endif

  bool MainWindow::event(QEvent *event)
  {
    // delayed initialization
    if(event->type() == QEvent::Polish) {

      // read settings
      readSettings();

      reloadTools();
      //if (d->toolSettingsDock)
      //  d->toolSettingsDock->hide();
      loadExtensions();

      // Check every menu for "extra" separators
      QList<QAction *> removeThese;
      foreach( QAction *menu, menuBar()->actions() ) {
        if (menu->menu()->actions().isEmpty())
          continue;

        menu->menu()->setSeparatorsCollapsible(true);
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

      if(!molecule())
        loadFile();

      // if we don't have a molecule then load a blank file
      d->initialized = true;
    }
#ifdef Q_WS_MAC
    else if(event->type() == QEvent::ActivationChange
            || event->type() == QEvent::WindowActivate) {
      updateWindowMenu();
    }
#endif
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

  GLWidget::ProjectionType MainWindow::projection() const
  {
     return d->glWidget->projection();
  }

  void MainWindow::setPerspective()
  {
    d->glWidget->setProjection(GLWidget::Perspective);
  }

  void MainWindow::setOrthographic()
  {
    d->glWidget->setProjection(GLWidget::Orthographic);
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
    ui.actionDebugInformation->setChecked(render);
    if (d->glWidget && d->glWidget->renderDebug() != render)
      d->glWidget->setRenderDebug(render);
  }

  bool MainWindow::quickRender() const
  {
    // Is the current widget using quick render?
    return d->glWidget->quickRender();
  }

  void MainWindow::setQuickRender(bool quick)
  {
    ui.actionQuickRender->setChecked(quick);
    if (d->glWidget && d->glWidget->quickRender() != quick)
      d->glWidget->setQuickRender(quick);
  }

  void MainWindow::showAllMolecules(bool)
  {
    if (!d->allMoleculesDialog)
      return;

    if (d->allMoleculesDialog->isVisible()) {
      d->allMoleculesDialog->hide();
    }
    else {
      d->allMoleculesDialog->show();
      d->allMoleculesDialog->raise();
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
    gl->setExtensions(d->pluginManager.extensions(this));
     */
    foreach (GLWidget *glwidget, d->glWidgets) {
      glwidget->reloadEngines();
      glwidget->setExtensions(d->pluginManager.extensions(this));
    }


    int count = d->enginesStacked->count();
    for (int i = 0; i < count; ++i) {
      QWidget *widget = d->enginesStacked->widget(i);
      foreach(QObject *object, widget->children()) {
        if (!object->isWidgetType())
          continue;
        EngineViewWidget *engineView = qobject_cast<EngineViewWidget *>(object);
        if (engineView)
          engineView->clear();
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

    // Call GLWidget::setToolGroup which will store a pointer to the navigate tool
    foreach(GLWidget *glWidget, d->glWidgets)
      glWidget->setToolGroup(d->toolGroup);

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
        << tr("Computational Chemistry Output") + " (*.out *.log *.dat *.output *.gamout *.g98 *.g03 *.nwo)"
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

      // If current window is not empty or has non-default file name,
      // check if we already have an open window with this file
      if ( !isDefaultFileName(d->fileName) ||
           (d->molecule && d->molecule->numAtoms()) ) {
        MainWindow *existing = findMainWindow( fileName );
        if ( existing ) {
          existing->show();
          existing->raise();
          existing->activateWindow();
          return;
        }
      }

      // if we have nothing open or modified
      bool loadInNewWindow = isWindowModified();
#ifdef Q_WS_MAC
      loadInNewWindow = !isDefaultFileName(d->fileName); // always load into a new window on Mac PR#2945916
#endif

      if ( !loadInNewWindow ) {
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
      } // if (loadInNewWindow)
    }
  }

  void MainWindow::openRecentFile()
  {
    QAction *action = qobject_cast<QAction *>( sender() );
    if ( action ) {
      openFile( action->data().toString() );
    }
  }

  bool MainWindow::isDefaultFileName(const QString fileName)
  {
    if (fileName.isEmpty())
      return true;

    QFileInfo fileInfo(fileName);
    return (fileInfo.baseName() == tr("untitled"));
  }

  QString MainWindow::defaultFileName()
  {
    return (tr("untitled") + ".cml");
  }

  bool MainWindow::loadFile(const QString &fileName,
                            OBFormat *format,
                            const QString &options)
  {
    // Now also sets the window icon on Mac
    // http://labs.trolltech.com/blogs/2007/12/28/spotlight-on-little-things/
    QString shownName = fileName;
    if(fileName.isEmpty()) {
      setMolecule(new Molecule(this));
      ui.actionAllMolecules->setEnabled(false); // only one molecule -- the blank slate
      return true;
    }

    statusBar()->showMessage(tr("Loading %1...", "%1 is a filename").arg(fileName),
                             5000 );

    QApplication::setOverrideCursor( Qt::WaitCursor );
    statusBar()->showMessage( tr("Loading %1...").arg(fileName), 5000 );

    QString formatType;
    if (format != NULL)
      formatType = format->GetID();

#ifdef WIN32
    // CML loading does not work on Windows with the new threaded code
    QFileInfo info(fileName);
    QString completeSuffix = info.completeSuffix();
    if (completeSuffix.contains("cml", Qt::CaseInsensitive) ||
        formatType.contains("cml", Qt::CaseInsensitive)) {
      Molecule *mol = MoleculeFile::readMolecule(fileName, formatType.trimmed());
      QApplication::restoreOverrideCursor();
      if (mol) {
        setFileName(fileName);
        setMolecule(mol);
      }
      else {
        QMessageBox::warning(this, tr("Avogadro"),
                             tr("Reading molecular file failed, file %1.").arg(fileName));
      return false;
      }
      ui.actionAllMolecules->setEnabled(false);
      return true;
    }
    // Other file types appear to work correctly - this should be fixed properly
#endif

    // This will work in a background thread -- we want to wait until the firstMolReady() signal appears
    d->moleculeFile = MoleculeFile::readFile(fileName, formatType.trimmed(),
                                             options, false);
    if (!d->moleculeFile)
      return false;

    if (!d->moleculeFile->isReady()) {
      // TODO: split into first molecule vs. whole file
      connect(d->moleculeFile, SIGNAL(ready()), this, SLOT(firstMolReady()));
      connect(d->moleculeFile, SIGNAL(ready()), this, SLOT(finishLoadFile()));

      if (!d->progressDialog) {
        d->progressDialog = new QProgressDialog(this);
        d->progressDialog->setRange(0,0); // indeterminate progress
        d->progressDialog->setLabelText(tr("Reading multi-molecule file. This may take a while..."));
        d->progressDialog->setWindowModality(Qt::WindowModal);
        d->progressDialog->setCancelButtonText(QString()); // no cancel button
      }
      d->progressDialog->show();
    } else {
      firstMolReady();
      finishLoadFile();
    }

    return true;
  }

  void MainWindow::check3DCoords(OBMol *obMolecule)
  {
    bool build = false;

    if (obMolecule->GetDimension() != 3) {

      // we may want to check with the user
      if (d->build3D == AskUser) {

        QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
        if (d->moleculeFile != NULL && d->moleculeFile->numMolecules() > 1)
          buttons = QMessageBox::YesToAll | QMessageBox::NoToAll | buttons;
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                                                       tr( "Avogadro" ),
                                                       tr("This file does not contain 3D coordinates."),
                                                       buttons,
                                                       this);

        msgBox->setInformativeText(tr("Do you want Avogadro to build a rough geometry?"));
        msgBox->setDefaultButton(QMessageBox::Yes);
        int retval = msgBox->exec();

        switch(retval) {
        case (QMessageBox::YesToAll):
          d->build3D = AlwaysBuild;
        case (QMessageBox::Yes):
          build = true;
          break;

        case (QMessageBox::NoToAll):
          d->build3D = NeverBuild;
          build = false;
          break;
        case (QMessageBox::No):
        default:
          QMessageBox::warning( this, tr( "Avogadro" ),
                                tr( "This file does not contain 3D coordinates.\n"
                                    "You may not be able to edit or view properly." ));
          build = false;
          break;
        }
        delete msgBox;
      }

      if (build || d->build3D == AlwaysBuild) {
        // In OB-2.2.2 and later, builder will use 2D coordinates if present
        OBBuilder builder;
        builder.Build(*obMolecule);
        obMolecule->AddHydrogens(); // Add some hydrogens before running force field

        OBForceField* pFF =  OBForceField::FindForceField("MMFF94")->MakeNewInstance();
        if (pFF && !pFF->Setup(*obMolecule)) {
          pFF = OBForceField::FindForceField("UFF")->MakeNewInstance();
          if (!pFF || !pFF->Setup(*obMolecule)) return; // can't do anything more
        }
        if (pFF) {
          pFF->ConjugateGradients(250, 1.0e-4);
          pFF->UpdateCoordinates(*obMolecule);
          delete pFF;
        }
      } // building geometry

    } // check 3D coordinates
  }

  void MainWindow::selectMolecule(int index, int)
  {
    if (!d->moleculeFile)
      return; // nothing to do

    qDebug() << " selectMolecule " << index;

    OBMol *obMolecule =  d->moleculeFile->OBMol(index);
    if (!obMolecule)
      return; // bad index
    check3DCoords(obMolecule);

    Molecule *mol = new Molecule;
    mol->setOBMol(obMolecule);
    mol->setFileName(d->molecule->fileName()); // copy the same filename
    setMolecule(mol);
  }

  void MainWindow::finishLoadFile()
  {
    if (!d->moleculeFile)
      return;

    if (d->moleculeFile->numMolecules() > 1)
      ui.actionAllMolecules->setEnabled(true); // only one molecule -- the blank slate

    if (!d->allMoleculesTable) {
      d->allMoleculesDialog = new QDialog(this);
      d->allMoleculesDialog->setWindowTitle(tr("Select Molecule to View"));

      QVBoxLayout *layout = new QVBoxLayout( d->allMoleculesDialog );
      layout->setMargin( 0 );
      layout->setSpacing( 6 );

      d->allMoleculesTable = new QTableWidget( d->allMoleculesDialog );
      d->allMoleculesTable->setAlternatingRowColors(true);
      d->allMoleculesTable->setSelectionMode(QAbstractItemView::SingleSelection);
      layout->addWidget(d->allMoleculesTable);

      // make sure the table stretches across the dialog as it resizes
      QHeaderView *horizontal = d->allMoleculesTable->horizontalHeader();
      horizontal->setResizeMode(QHeaderView::Stretch);
      QHeaderView *vertical = d->allMoleculesTable->verticalHeader();
      vertical->setResizeMode(QHeaderView::Stretch);

    }

    disconnect(d->allMoleculesTable, 0, this, 0);
    d->allMoleculesTable->clear();
    d->allMoleculesTable->setRowCount(d->moleculeFile->numMolecules());
    d->allMoleculesTable->setColumnCount(1);

    QStringList columnLabels, rowLabels;
    columnLabels << tr("Molecule Title");
    d->allMoleculesTable->setHorizontalHeaderLabels(columnLabels);
    int molecule = 0;
    foreach(const QString &title, d->moleculeFile->titles()) {
      QTableWidgetItem* newItem = new QTableWidgetItem(title);
      d->allMoleculesTable->setItem(molecule, 0, newItem);
      //      qDebug() << " molecule: " << molecule << title;
      rowLabels << QString("%L1").arg(++molecule);
    }
    d->allMoleculesTable->setCurrentCell(0, 0, QItemSelectionModel::ClearAndSelect);
    connect(d->allMoleculesTable, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(selectMolecule(int, int)));
  }

  void MainWindow::firstMolReady()
  {
    if (d->moleculeFile == NULL)
      return;
    if (d->progressDialog) {
      d->progressDialog->reset();
#ifdef Q_WS_MAC
      d->progressDialog->deleteLater();
      d->progressDialog = 0;
#endif
    }

    ui.actionAllMolecules->setEnabled(false); // only one molecule right now

    QString errors = d->moleculeFile->errors();
    OBMol *obMolecule = d->moleculeFile->OBMol();
    if (errors.isEmpty() && obMolecule != NULL) { // successful read

      qDebug() << " read " << d->moleculeFile->numMolecules() << " molecules.";

      // e.g. SMILES or MDL molfile, etc.
      check3DCoords(obMolecule);

      Molecule *mol = new Molecule;
      mol->setOBMol(obMolecule);
      mol->setFileName(d->moleculeFile->fileName());
      if (d->moleculeFile->isConformerFile()) {
        // add in the conformers
        mol->setAllConformers(d->moleculeFile->conformers());
        qDebug() << " # of conformers " << mol->numConformers();
      }

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

      QString status;
      QTextStream( &status ) << tr("Atoms: ") << d->molecule->numAtoms() <<
        tr(" Bonds: ") << d->molecule->numBonds();
      statusBar()->showMessage( status, 5000 );
    }
    else { // errors
      // @TODO: show errors in Messages Tab
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
                            isDefaultFileName(d->fileName) ? tr("Save...") : tr("Save"));

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
    if (maybeSave())
      loadFile();
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
      emit(windowClosed());
      writeSettings();
      event->accept();
    } else {
      event->ignore();
    }
  }

  bool MainWindow::save()
  {
    // we can't safely save to a gzipped file
    if ( !QFileInfo(d->fileName).isReadable()
        || isDefaultFileName(d->fileName)
        || d->fileName.endsWith(".gz", Qt::CaseInsensitive)) {
      return saveAs();
    }
    else
      return saveFile( d->fileName );
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

    // delete the MoleculeFile before calling saveFile.
    // We don't want to replace the molecule but save it
    // to another file.
    if (d->moleculeFile) {
      delete d->moleculeFile;
      d->moleculeFile = 0;
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
      QString error;
      bool result = MoleculeFile::writeMolecule(d->molecule, fileName,
                                                formatType.trimmed(), "",
                                                &error);
      if (!result) { // There was an error saving the file - inform the user
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("Avogadro"), error);
        return false;
      }
      else {
        QApplication::restoreOverrideCursor();
        setWindowModified(false);
        return true;
      }
    }
    else { /// FIXME Add error checking
      if (d->moleculeFile->isConformerFile()) {
        MoleculeFile::writeConformers(d->molecule, fileName, formatType.trimmed());
        QApplication::restoreOverrideCursor();
        setWindowModified(false);
        return true;
      }
      else { /// FIXME Add error checking
        // use MoleculeFile to save just the current slice of the file
        d->moleculeFile->replaceMolecule(d->currentIndex, d->molecule, fileName);
        QApplication::restoreOverrideCursor();
        setWindowModified(false);
        return true;
      }
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
    return false;
  }

  void MainWindow::setIgnoreConfig(bool noConfig)
  {
    m_ignoreConfig = noConfig;
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

  void MainWindow::revert()
  {
    if ( !isDefaultFileName(d->fileName) ) {
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
    settings.setValue("recentFileList", files);

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
      if (! isDefaultFileName(widget->d->fileName) )
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
  void MainWindow::openManualURL() const
  {
    QDesktopServices::openUrl(QUrl("http://manual.avogadro.cc/"));
  }

  void MainWindow::openForumURL() const
  {
    QDesktopServices::openUrl(QUrl("http://discuss.avogadro.cc/"));
  }

  void MainWindow::openTutorialURL() const
  {
    QDesktopServices::openUrl(QUrl("http://avogadro.cc/tutorials"));
  }

  void MainWindow::openFAQURL() const
  {
    QDesktopServices::openUrl(QUrl("http://avogadro.cc/FAQ"));
  }

  void MainWindow::openWebsiteURL() const
  {
    QDesktopServices::openUrl(QUrl("http://avogadro.cc/"));
  }

  void MainWindow::openReleaseNotesURL() const
  {
    QDesktopServices::openUrl(QUrl( "http://avogadro.cc/Avogadro_" + QString(VERSION) ));
  }

  void MainWindow::openBugURL() const
  {
    QDesktopServices::openUrl(QUrl("http://github.com/cryos/avogadro/issues"));
  }

  void MainWindow::setView(int index)
  {
    QWidget *widget = d->centralTab->widget(index);
    foreach(QObject *object, widget->children()) {
      GLWidget *glWidget = qobject_cast<GLWidget *>(object);
      if (glWidget) {
        d->glWidget = glWidget;
        int idx =d->glWidgets.indexOf(glWidget);
        d->enginesStacked->setCurrentIndex(idx);
        ui.actionDisplayAxes->setChecked(renderAxes());
        ui.actionDebugInformation->setChecked(renderDebug());
        ui.actionQuickRender->setChecked(quickRender());
        break;
      }
    }
  }

  void MainWindow::glWidgetActivated(GLWidget *glWidget)
  {
    if (d->glWidget == glWidget)
      return;

    d->glWidget = glWidget;

    int index = d->glWidgets.indexOf(glWidget);
    d->enginesStacked->setCurrentIndex(index);
    ui.actionDisplayAxes->setChecked(renderAxes());
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

    bool validMol = false;
    if ( conv.ReadString( &newMol, text.constData() ) // Can we read with OB formats?
         && newMol.NumAtoms() != 0 ) {
      validMol = true;
    }

    if (!validMol) { // We failed as an authentic format, try annulen's heuristics
      validMol = parseText(&newMol, QString(text));
    }

    if (validMol && newMol.NumAtoms() == 0)
      return false;

    // We've got something we can paste
    /*
    vector3 offset; // small offset so that pasted mols don't fall on top
    offset.randomUnitVector();
    offset *= 0.1;
    newMol.Translate(offset);
    */

    Molecule newMolecule;
    newMolecule.setOBMol(&newMol);
    PasteCommand *command = new PasteCommand(d->molecule, newMolecule, d->glWidget);
    d->undoStack->push(command);
    d->toolGroup->setActiveTool("Manipulate"); // set the tool to manipulate, so we can immediate move the selection
    return true;
  }

  int GetAtomicNum(string name, int &iso)
  {
    int n = OpenBabel::etab.GetAtomicNum(name.c_str(), iso);
    if (iso != 0)
      return 0;  // "D" ot "T"
    if (n != 0)
      return n;  // other element symbols

    // not match => we've got IUPAC name

    /*vector<OBElement*>::iterator i;
      for (i = _element.begin();i != _element.end();++i)
      if (name == (*i)->GetSymbol())
      return((*i)->GetAtomicNum());*/

    for (unsigned int i=0; i<etab.GetNumberOfElements(); i++)
      if (!QString::compare(name.c_str(), etab.GetName(i).c_str(), Qt::CaseInsensitive))
	      return i;

    if (!QString::compare(name.c_str(), "Deuterium", Qt::CaseInsensitive))
      {
        iso = 2;
        return(1);
      }
    else if (!QString::compare(name.c_str(), "Tritium", Qt::CaseInsensitive))
      {
        iso = 3;
        return(1);
      }
    else
      iso = 0;
    return(0);
  }

  bool MainWindow::parseText(OBMol *mol, const QString coord)
  {
    QStringList coordStrings = coord.split(QRegExp("\n"));

    double k = 1.0; // ANGSTROM -- set to 0.529 for Bohr

    // Guess format

    // split on any non-word symbol, except '.'
    QStringList data = coordStrings.at(0).trimmed().split(QRegExp("\\s+|,|;"));
    //QList<double>
    // Format definition, will be used for parsing
    int NameCol=-1, Xcol=-1, Ycol=-1, Zcol=-1;
    QString format("");
    double b;
    bool ok;
    for (int i=0; i<data.size(); i++)
      {
        if (data.at(i) == "") continue;

        data.at(i).toInt(&ok);
        if (ok)
          {
            format += "i";
            continue;
          }

        b = data.at(i).toDouble(&ok);
        if (ok)
          {
            if ((int)b == b && b!=0)
              format += "i";	// non-zero integer found - not likely to be coordinate
            else
              format += "d";
          }
        else
          format += "s";
      }

    qDebug() << "Format is: " << format;

    if (format.length() < 4)
      return false; // invalid format

    if (format == "iddd") // special XYZ variant
      {
        NameCol=0;
        Xcol=1;
        Ycol=2;
        Zcol=3;
      }
    else // more columns
      {
        for (int i=0; i<format.length(); i++)
          {
            //if (format.at(i) == 'i')
            //continue; // nothing valuable

            if ((format.at(i)=='d') || (format.length()==4 && format.at(i)=='i'))
              {
                // double
                if (Xcol==-1)
                  {
                    Xcol=i;
                    continue;
                  }
                if (Ycol==-1)
                  {
                    Ycol=i;
                    continue;
                  }
                if (Zcol==-1)
                  {
                    Zcol=i;
                    continue;
                  }
                continue; // nothing valuable
              }

            if (format.at(i) == 's')
              {
                // string
                if (NameCol != -1)  // just found
                  continue;

                // Try to find element name or symbol inside it
                int n,iso;
                QString s = data.at(i);
                while (s.length()!=0)  // recognize name with number
                  {
                    iso = 0;
                    n = GetAtomicNum(s.toStdString(), iso);
                    if (iso != 0)
                      n = 1;

                    if (n!=0)
                      {
                        NameCol=i;
                        break;
                      }
                    else
                      s.chop(1);
                  }
              }
            continue;
          }
      }

    if((NameCol==-1) || (Xcol==-1) || (Ycol==-1) || (Zcol==-1))
      return false;

    // Read and apply coordinates
    mol->BeginModify();
    for (int N=0; N<coordStrings.size(); N++)
      {
        if (coordStrings.at(N) == "") continue;

        OBAtom *atom  = mol->NewAtom();
        QStringList s_data = coordStrings.at(N).trimmed().split(QRegExp("\\s+|,|;"));
        if (s_data.size() != data.size())
          return false;
        for (int i=0; i<s_data.size(); i++)
          {
            double x(0.0), y(0.0), z(0.0);
            int _n(0),_iso;
            bool ok = true;
            if (i == Xcol)
              x = s_data.at(i).toDouble(&ok);
            else if (i == Ycol)
              y = s_data.at(i).toDouble(&ok);
            else if (i == Zcol)
              z = s_data.at(i).toDouble(&ok);
            else if ((i == NameCol)	&& (format == "iddd"))
              _n = s_data.at(i).toInt(&ok);
            else if (i == NameCol)
              {

                // Try to find element name or symbol inside it

                QString _s = s_data.at(i);
                while (_s.length()!=0)  // recognize name with number
                  {
                    _iso=0;
                    _n = GetAtomicNum(_s.toStdString(), _iso);
                    if (_iso != 0)
                      _n = 1;

                    if (_n!=0)
                      break;
                    else
                      _s.chop(1);
                  }
                if (_n==0)
                  return false;
              }
            if (!ok) return false;

            atom->SetAtomicNum(_n);
            atom->SetVector(x*k,y*k,z*k); //set coordinates
          }
      }
    mol->EndModify();
    mol->ConnectTheDots();
    mol->PerceiveBondOrders();

    //qDebug() << "molecule updated";
    return true;
  }

  /// @todo this should live in a UnitCell class eventually
  // Stable sort both ids and coords together to group all entries in
  // ids together. Entries are not sorted in any particular order,
  // just grouped. uniqIds and idCounts will contain a unique list of
  // all ids in same order as in the sorted ids and a list containing
  // how many of each id is in ids, respectively.
  void poscarSort(QList<QString> *ids,
                  QList<Eigen::Vector3d> *coords,
                  QList<QString> *uniqueIds,
                  QList<unsigned int> *idCounts)
  {
    Q_ASSERT(ids->size() == coords->size());
    // Get unique list of ids and count them
    uniqueIds->clear();
    idCounts->clear();
    for (QStringList::const_iterator
           it = ids->constBegin(),
           it_end = ids->constEnd();
         it != it_end; ++it) {
      int ind = uniqueIds->indexOf(*it);
      if (ind != -1) {
        ++((*idCounts)[ind]);
      }
      else {
        uniqueIds->append(*it);
        idCounts->append(1);
      }
    }

    // Sort lists
    QString curId;
    QStringList::iterator idit;
    QList<Eigen::Vector3d>::iterator coordit;
    unsigned int sorted = 0;
    for (int uniqInd = 0; uniqInd < uniqueIds->size();
         ++uniqInd) {
      curId = (*uniqueIds)[uniqInd];
      unsigned int found = 0;
      unsigned int count = idCounts->at(uniqInd);
      idit = ids->begin() + sorted;
      coordit = coords->begin() + sorted;
      while (found < count) {
        // Should never reach the end
        Q_ASSERT(idit != ids->end());
        Q_ASSERT(coordit != coords->end());
        if (idit->compare(curId) == 0) {
          qSwap(*idit, (*ids)[sorted]);
          qSwap(*coordit, (*coords)[sorted]);
          ++found;
          ++sorted;
        }
        ++idit;
        ++coordit;
      }
    }
  }

  /// @todo this should live in a UnitCell class eventually
  QString getPOSCAR(Molecule *mol)
  {
    OBUnitCell *cell = mol->OBUnitCell();
    if (!cell) {
      return "";
    }

    QList<Atom*> atoms = mol->atoms();

    // Atomic symbols:
    QStringList ids;
    for (QList<Atom*>::const_iterator it = atoms.constBegin(),
           it_end = atoms.constEnd(); it != it_end; ++it) {
      ids << OpenBabel::etab.GetSymbol((*it)->atomicNumber());
    }

    // Fractional coordinates
    QList<Vector3d> fcoords;
    const Vector3d *eigenptr;
    vector3 obtmp;
    for (QList<Atom*>::const_iterator it = atoms.begin(),
           it_end = atoms.end(); it != it_end; ++it) {
      // Convert eigen to OB
      eigenptr = (*it)->pos();
      obtmp.x() = eigenptr->x();
      obtmp.y() = eigenptr->y();
      obtmp.z() = eigenptr->z();
      // Convert cartesian -> fractional
      obtmp = cell->CartesianToFractional(obtmp);
      // Back to eigen
      fcoords << Vector3d(obtmp.x(), obtmp.y(), obtmp.z());
    }

    // For sorting
    QStringList uniqueIds;
    QList<unsigned int> idCounts;

    Q_ASSERT(fcoords.size() == ids.size());

    poscarSort(&ids, &fcoords, &uniqueIds, &idCounts);

    Q_ASSERT(uniqueIds.size() == idCounts.size());

    QString poscar;

    // Comment line: composition
    for (unsigned int i = 0;
         i < static_cast<unsigned int>(uniqueIds.size());
         ++i) {
      poscar += QString("%1%2 ").arg(uniqueIds[i]).arg(idCounts[i]);
    }
    poscar += "\n";
    // Scaling factor. Just 1.0
    poscar += QString::number(1.0);
    poscar += "\n";
    // Unit Cell Vectors
    std::vector< OpenBabel::vector3 > vecs = cell->GetCellVectors();
    for (unsigned int i = 0; i < vecs.size(); i++) {
      OpenBabel::vector3 &vec = vecs[i];
      // Remove negative zeros
      if (fabs(vec.x()) < 1e-10) {
        vec.x() = 0.0;
      }
      if (fabs(vec.y()) < 1e-10) {
        vec.y() = 0.0;
      }
      if (fabs(vec.z()) < 1e-10) {
        vec.z() = 0.0;
      }
      poscar += QString("  %1 %2 %3\n")
        .arg(vec.x(), 12, 'f', 8)
        .arg(vec.y(), 12, 'f', 8)
        .arg(vec.z(), 12, 'f', 8);
    }
    // Number of each type of atom
    for (int i = 0; i < idCounts.size(); i++) {
      poscar += QString::number(idCounts.at(i)) + " ";
    }
    poscar += "\n";
    // Use fractional coordinates:
    poscar += "Direct\n";
    // Coordinates of each atom
    for (int i = 0; i < fcoords.size(); i++) {
      Eigen::Vector3d &fcoord = fcoords[i];
      // Remove negative zeros
      if (fabs(fcoord.x()) < 1e-10) {
        fcoord.x() = 0.0;
      }
      if (fabs(fcoord.y()) < 1e-10) {
        fcoord.y() = 0.0;
      }
      if (fabs(fcoord.z()) < 1e-10) {
        fcoord.z() = 0.0;
      }
      poscar += QString("  %1 %2 %3\n")
        .arg(fcoord.x(), 12, 'f', 8)
        .arg(fcoord.y(), 12, 'f', 8)
        .arg(fcoord.z(), 12, 'f', 8);
    }

    return poscar;
  }

  // Helper function -- works for "cut" or "copy"
  // FIXME add parameter to set "Copy" or "Cut" in messages
  QMimeData* MainWindow::prepareClipboardData(PrimitiveList selectedItems,
                                              const char* format)
  {
    QMimeData *mimeData = new QMimeData;
    QImage clipboardImage;

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

      // Copy unit cell, if available
      if (d->molecule->OBUnitCell()) {
        moleculeCopy->setOBUnitCell
          (new OBUnitCell(*(d->molecule->OBUnitCell())));
      }
    } // should now have a copy of our selected fragment

    if (!format) {
      // Default:
      // we also save an image for copy/paste to office programs, presentations, etc.
      // we do this first, so we can embed the molecular data too
      d->glWidget->raise();
      d->glWidget->repaint();
      if (QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
        clipboardImage = d->glWidget->grabFrameBuffer( true );
      } else {
        QPixmap pixmap = QPixmap::grabWindow( d->glWidget->winId() );
        clipboardImage = pixmap.toImage();
      }
    }

    OBConversion conv;
    if (format) {
      // The user specified a format (e.g., SMILES) so use it
      OBFormat *pFormat = conv.FindFormat(format);
      if (!pFormat || !conv.SetOutFormat(pFormat)) {
        statusBar()->showMessage( tr( "Copy failed (format unavailable)." ), 5000 );
        return NULL; // nothing in it yet
      }

      OBMol obmol = moleculeCopy->OBMol();
      string output = conv.WriteString(&obmol);
      QString copyData(output.c_str());
      mimeData->setText(copyData.trimmed()); // remove any newlines or whitespace
    }
    else {
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

      // Copy XYZ coordinates to the text selection buffer for finite
      // systems, or POSCAR if a unit cell is available
      OBUnitCell *cell = moleculeCopy->OBUnitCell();
      if (!cell) {
        OBFormat *xyzFormat = conv.FindFormat("xyz");
        if ( xyzFormat && conv.SetOutFormat(xyzFormat)) {
          output = conv.WriteString(&obmol);
          copyData = output.c_str();
          mimeData->setText(QString(copyData));
        }
      }
      else {
        QString poscar = getPOSCAR(moleculeCopy);
        mimeData->setText(poscar);
      }

      // save the image to the clipboard too
      mimeData->setImageData(clipboardImage);
    }

    // need to free our temporary moleculeCopy
    if (!selectedItems.isEmpty()) {
      delete moleculeCopy;
    }

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
      // For X11 middle click
      if (QApplication::clipboard()->supportsSelection()) {
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
      }
    }
  }

  void MainWindow::copyAsSMILES()
  {
    QMimeData *mimeData = prepareClipboardData(d->glWidget->selectedPrimitives(),
                                               "smi");

    if ( mimeData ) {
      QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
      // For X11 middle click
      if (QApplication::clipboard()->supportsSelection()) {
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
      }
    }
  }

  void MainWindow::copyAsInChI()
  {
    QMimeData *mimeData = prepareClipboardData(d->glWidget->selectedPrimitives(),
                                               "inchi");

    if ( mimeData ) {
      QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
      // For X11 middle click
      if (QApplication::clipboard()->supportsSelection()) {
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
      }
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
    d->centralTab->setTabsClosable(d->centralTab->count() > 1);
    ui.actionCloseView->setEnabled(true);
    ui.actionDetachView->setEnabled(true);
    ui.actionDisplayAxes->setChecked(gl->renderAxes());
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
    d->centralTab->setTabsClosable(d->centralTab->count() > 1);
    ui.actionCloseView->setEnabled(true);
    ui.actionDetachView->setEnabled(true);
    ui.actionDisplayAxes->setChecked(gl->renderAxes());
    ui.actionDebugInformation->setChecked(gl->renderDebug());
    ui.actionQuickRender->setChecked(gl->quickRender());

    writeSettings();
  }

  void MainWindow::detachView()
  {
    if (d->centralLayout->count() == 1) // Don't close/detach the final view
      return;
    // Get the GLWidget of the current view, close in in the tabs
    QWidget *widget = d->centralTab->currentWidget();
    foreach(QObject *object, widget->children()) {
      GLWidget *glWidget = qobject_cast<GLWidget *>(object);
      if (glWidget) {
        int index = d->centralTab->currentIndex();
        d->centralTab->removeTab(index);

        for (int count=d->centralTab->count(); index < count; ++index) {
          d->centralTab->setTabText(index, tr("View %1", "View number (from 1 on)")
                                           .arg(index +1));
        }
        // Ensure that actions are enabled/disabled appropriately.
        ui.actionCloseView->setEnabled(d->centralTab->count() != 1);
        ui.actionDetachView->setEnabled(d->centralTab->count() != 1);
        d->centralTab->setTabsClosable(d->centralTab->count() != 1);
        // Set up the detached viwe
        DetachedView *view = new DetachedView(glWidget);
        view->setWindowTitle(tr("Avogadro: Detached View"));
        view->resize(glWidget->size());
        view->show();

        connect(this, SIGNAL(windowClosed()), view, SLOT(mainWindowClosed()));
      }
    }
  }

  void MainWindow::closeView()
  {
    closeView(d->centralTab->currentIndex());
  }

  void MainWindow::closeView(int index)
  {
    if (d->centralLayout->count() == 1) // Don't close the final view
      return;

    QWidget *widget = d->centralTab->widget(index);
    foreach( QObject *object, widget->children() ) {
      GLWidget *glWidget = qobject_cast<GLWidget *>(object);
      if (glWidget) {
        d->centralTab->removeTab(index);

        // delete the engines list for this GLWidget
        QWidget *widget = d->enginesStacked->widget( index );
        d->enginesStacked->removeWidget( widget );
        delete widget;

        for (int count=d->centralTab->count(); index < count; ++index)
          d->centralTab->setTabText(index, tr("View %1").arg(index + 1));
        d->glWidgets.removeAll( glWidget );
        delete glWidget;
        ui.actionCloseView->setEnabled(d->centralTab->count() != 1);
        ui.actionDetachView->setEnabled(d->centralTab->count() != 1);
        d->centralTab->setTabsClosable(d->centralTab->count() != 1);
      }
    }
    setView( d->centralTab->currentIndex());
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
    Projective3d goal(linearGoal);

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
    if( !d->molecule || d->molecule->numAtoms() >= 1000 ) {
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

  void MainWindow::alignViewToAxes()
  {
    // do nothing if there is a timer running
    if(d->centerTimer)
      return;

    Camera * camera = d->glWidget->camera();
    if(!camera)
      return;

    // determine our goal matrix
    Matrix3d linearGoal = Matrix3d::Identity();

    // calculate the translation matrix
    Projective3d goal(linearGoal);

    goal.pretranslate(- 3.0 * (d->glWidget->radius() + CAMERA_NEAR_DISTANCE) * Vector3d::UnitZ());

    // Support centering on a selection
    QList<Primitive*> selectedAtoms = d->glWidget->selectedPrimitives().subList(Primitive::AtomType);
    if (selectedAtoms.isEmpty()) { // no selected atoms, center on 0,0,0
      goal.translate( Vector3d::Zero() );
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
    if( !d->molecule || d->molecule->numAtoms() >= 1000 ) {
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
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();
  }

  void MainWindow::fullScreen()
  {
    if ( !this->isFullScreen() ) {
      ui.actionFullScreen->setText( tr( "Normal Size" ) );
      #if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
        ui.actionFullScreen->setIcon( QIcon::fromTheme("view-restore", QIcon( ":/icons/view-restore.png" )) );
      #else
        ui.actionFullScreen->setIcon( QIcon( ":/icons/view-restore.png" ) );
      #endif
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
      #if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
        ui.actionFullScreen->setIcon( QIcon::fromTheme("view-fullscreen", QIcon( ":/icons/view-fullscreen.png" )) );
      #else
        ui.actionFullScreen->setIcon( QIcon( ":/icons/view-fullscreen.png" ) );
      #endif
      ui.fileToolBar->setVisible(d->fileToolbar);
      statusBar()->setVisible(d->statusBar);
    }
  }

  void MainWindow::resetDisplayTypes()
  {
    d->glWidget->loadDefaultEngines();
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
    connect( ui.actionSMILES, SIGNAL( triggered() ), this, SLOT( copyAsSMILES() ) );
    connect( ui.actionInChI, SIGNAL( triggered() ), this, SLOT( copyAsInChI() ) );
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
    connect( ui.actionAlignViewToAxes, SIGNAL( triggered() ),
             this, SLOT( alignViewToAxes() ) );
    connect( ui.actionFullScreen, SIGNAL( triggered() ),
             this, SLOT( fullScreen() ) );
    connect( ui.actionResetDisplayTypes, SIGNAL( triggered() ),
             this, SLOT( resetDisplayTypes() ) );
    connect( ui.actionSetBackgroundColor, SIGNAL( triggered() ),
             this, SLOT( setBackgroundColor() ) );
    connect( ui.actionPerspective, SIGNAL( triggered() ),
             this, SLOT( setPerspective() ) );
    connect( ui.actionOrthographic , SIGNAL( triggered() ),
             this, SLOT( setOrthographic() ) );
    connect(ui.actionDisplayAxes, SIGNAL(triggered(bool)),
            this, SLOT(setRenderAxes(bool)));
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

    connect( ui.actionAvogadro_Help, SIGNAL( triggered() ),
             this, SLOT( openManualURL() ));
    connect( ui.actionTutorials, SIGNAL( triggered() ),
             this, SLOT( openTutorialURL() ));
    connect( ui.actionFAQ, SIGNAL( triggered() ),
             this, SLOT( openFAQURL() ) );
    connect( ui.actionAvogadro_Forum, SIGNAL( triggered() ),
             this, SLOT( openForumURL() ));
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

  void MainWindow::setMolecule(Molecule *molecule, int options)
  {
    if (d->molecule && options & Extension::DeleteOld) {
      disconnect(d->molecule, 0);
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
      setWindowFilePath(defaultFileName());

    emit moleculeChanged(molecule);

    connect(d->molecule, SIGNAL(primitiveAdded(Primitive *)),
             this, SLOT(documentWasModified()));
    connect(d->molecule, SIGNAL(primitiveUpdated(Primitive *)),
             this, SLOT(documentWasModified() ) );
    connect(d->molecule, SIGNAL(primitiveRemoved(Primitive *)),
             this, SLOT(documentWasModified()));
    connect(d->molecule, SIGNAL(updated()), this, SLOT(documentWasModified()));

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

  void MainWindow::setFileName(const QString &fileName)
  {
    if ( fileName.isEmpty() ) {
      d->fileName.clear();
      setWindowTitle( tr( "[*]Avogadro" , "[*] indicates that this is a modified view...." ) );
    }
    else {
      QFileInfo fileInfo(fileName);
      d->fileName = fileInfo.canonicalFilePath();
      d->fileDialogPath = fileInfo.absolutePath();
      setWindowTitle( tr( "%1[*] - %2", "Window title: %1 is file name, %2 is Avogadro" ).arg( fileInfo.fileName() )
          .arg( tr( "Avogadro" ) ) );

      // Check that the canonical file path exists - only update recent files
      // if it does. Should prevent empty list items on initial open etc.
      if (! isDefaultFileName(d->fileName) ) {
        QSettings settings; // already set up properly via main.cpp
        QStringList files = settings.value("recentFileList").toStringList();
        files.removeAll(d->fileName);
        files.prepend(d->fileName);
        while (files.size() > maxRecentFiles)
          files.removeLast();

        settings.setValue("recentFileList", files);
      }

      // Set the fileName for the actual molecule too
      if (d->molecule)
        d->molecule->setFileName(d->fileName);
    }

    foreach( QWidget *widget, QApplication::topLevelWidgets() ) {
      MainWindow *mainWin = qobject_cast<MainWindow *>( widget );
      if (mainWin)
        mainWin->updateRecentFileActions();
    }
  }

  void MainWindow::updateRecentFileActions()
  {
    QSettings settings; // set up project and program properly in main.cpp
    QStringList files = settings.value("recentFileList").toStringList();

    int originalNumRecentFiles = files.size();

    // Remove any duplicate or empty entries from the list
    files.removeDuplicates();
    files.removeAll(QString());
    // Now remove any entries which do not exist
    for(int i = files.size()-1; i >= 0; --i) {
      QFileInfo fileInfo(files[i]);
      if (!QFile(fileInfo.absoluteFilePath()).exists())
        files.removeAt(i);
    }

    int numRecentFiles = qMin(files.size(),
                              static_cast<int>(maxRecentFiles));

    for (int i = 0; i < numRecentFiles; ++i) {
      d->actionRecentFile[i]->setText(QFileInfo(files[i]).fileName());
      d->actionRecentFile[i]->setData(files[i]);
      d->actionRecentFile[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < maxRecentFiles; ++j)
      d->actionRecentFile[j]->setVisible(false);

    // If we had to prune the list, then save the cleaned list
    if (originalNumRecentFiles != numRecentFiles)
      settings.setValue("recentFileList", files);
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

    if (m_ignoreConfig)
      settings.clear();

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

    for(int i = 0; i < count; i++) {
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
    ui.actionDebugInformation->setChecked(renderDebug());
    ui.actionQuickRender->setChecked(quickRender());

    // Set the initial state of the action group for "View > Projection"
    switch(projection()) {
    case GLWidget::Perspective:
      ui.actionPerspective->setChecked(true);
      break;
    case GLWidget::Orthographic:
      ui.actionOrthographic->setChecked(true);
      break;
    }

    ui.actionCloseView->setEnabled(count > 1);
    ui.actionDetachView->setEnabled(count > 1);
    d->centralTab->setTabsClosable(count > 1);

#ifdef ENABLE_UPDATE_CHECKER
    // Load the updated version configuration settings and then run it
    if (m_updateCheck) {
      m_updateCheck->readSettings(settings);
      m_updateCheck->checkForUpdates();
    }
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
    if (m_updateCheck)
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
            bool found = false;
            foreach( QAction *menu, path->actions() ) {
              if ( menu->text() == menuPath.at( i ) ) {
                nextPath = menu->menu();
                found = true;
                break;
              }
            } // end checking menu items

            if ( !found ) {
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

      // This is the preferred method for adding a dockwidget to the main
      // window
      if (extension->numDockWidgets() != 0) {
        QList<DockWidget *> widgets = extension->dockWidgets();
        for (QList<DockWidget*>::const_iterator it = widgets.constBegin(),
             it_end = widgets.constEnd(); it != it_end; ++it) {
          if (!this->restoreDockWidget(*it)) {
            // No restore state -- use the preferred area
            this->removeDockWidget((*it));
            this->addDockWidget((*it)->preferredWidgetDockArea(), *it);
          }
          (*it)->hide();
          ui.menuToolbars->addAction((*it)->toggleViewAction());
        }
      }
      else {
        // These are deprecated methods for adding dock widgets.
        QDockWidget *dockWidget = extension->dockWidget();
        if(dockWidget) {
          Qt::DockWidgetArea area = Qt::RightDockWidgetArea;
          DockExtension *dock = qobject_cast<DockExtension *>(extension);
          if (dock) {
            area = dock->preferredDockArea();
          }
          qDebug() << "dev warning: Extension" << extension->name()
                   << "is using a deprecated DockWidget loading method. "
                      "See Extension::dockWidgets() documentation.";
          if (!restoreDockWidget(dockWidget)) {
            // No restore state -- use the preferred area
            removeDockWidget(dockWidget);
            addDockWidget(area, dockWidget);
          }
          dockWidget->hide();
          ui.menuToolbars->addAction(dockWidget->toggleViewAction());
        }
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
        if (menuItem->menu() == 0) { // ignore submenus
          menuItem->setEnabled(false);
        } else { // submenu items

          // Don't modify the "open recent" sub-menu
          if (menuItem->menu() == ui.menuOpenRecent)
            continue;

          foreach( QAction *subMenuItem, menuItem->menu()->actions() ) {
            subMenuItem->setEnabled(false); // disable submenus
          }
        }
      }
    }

    // Now enable key menu items -- new, open, open recent, quit, etc.
    ui.actionAbout->setEnabled( true );
    ui.actionNew->setEnabled( true );
    ui.actionOpen->setEnabled( true );
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
        if (menuItem->menu() != 0) { // submenu
          foreach( QAction *subMenuItem, menuItem->menu()->actions() ) {
            subMenuItem->setEnabled(true); // re-enable submenus
          }
        }
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
    if(!d->glWidget) {
      gl = new GLWidget(this);
      d->glWidget = gl;
    }
    else
      gl = new GLWidget( d->glWidget->format(), this, d->glWidget );

    // Connect up a few signals and slots we need
    connect( this, SIGNAL( moleculeChanged( Molecule * ) ),
             gl, SLOT( setMolecule( Molecule * ) ) );
    connect(gl, SIGNAL(activated(GLWidget *)),
            this, SLOT(glWidgetActivated(GLWidget *)));

    gl->setMolecule(d->molecule);
    gl->setObjectName(QString::fromUtf8("glWidget"));
    gl->setUndoStack( d->undoStack );
    gl->setToolGroup( d->toolGroup );
    d->glWidgets.append(gl);

    // Set the extensions (needed for Extension::paint)
    gl->setExtensions(d->pluginManager.extensions(this));

    // engine list wiget contains all the buttons too
    QWidget *engineListWidget = new QWidget(ui.enginesWidget);
    QVBoxLayout *vlayout = new QVBoxLayout(engineListWidget);

    EngineViewWidget *engineView = new EngineViewWidget(gl, engineListWidget);
    vlayout->addWidget(engineView);
    connect(engineView, SIGNAL(settings(Engine *)), this,
            SLOT(engineSettingsClicked(Engine *)));

    // Then a row of add, duplicate, remove
    QHBoxLayout *hlayout = new QHBoxLayout();
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

    connect(engineView, SIGNAL(clicked(Engine *)),
        this, SLOT(engineClicked(Engine *)));

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

  void MainWindow::engineSettingsClicked(Engine *engine)
  {
    if (!engine)
      return;
    Engine *selectedEngine = engine;

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
    Engine *engine = d->currentSelectedEngine;

    if(engine) {
      Engine *newEngine = engine->clone();
      PrimitiveList list = d->glWidget->selectedPrimitives();
      if(list.size()) {
        newEngine->setPrimitives(list);
      }
      newEngine->setAlias(newEngine->alias() + tr(" copy"));
      d->glWidget->addEngine(newEngine);
    }
  }

  void MainWindow::removeEngineClicked()
  {
    Engine *engine = d->currentSelectedEngine;

    if(engine) {
      d->glWidget->removeEngine(engine);
      d->engineSettingsWindows.remove(engine);
    }
  }

  void MainWindow::engineClicked(Engine *engine)
  {
    if (!engine)
      return;
    d->currentSelectedEngine = engine;
  }

  void MainWindow::toggleToolSettingsDock()
  {
    d->toolSettingsDock->setVisible(! d->toolSettingsDock->isVisible());
  }

  void MainWindow::toggleEngineSettingsDock()
  {
    ui.enginesDock->setVisible(! ui.enginesDock->isVisible() );
  }

  QStringList MainWindow::pluginSearchDirs()
  {
    // This is where we compile a list of directories that will be searched
    // for plugins. This is quite dependent up on operating system

    // Environment variables can override default paths
    foreach (const QString &variable, QProcess::systemEnvironment()) {
      if(variable.startsWith("AVOGADRO_PLUGINS=")) {
        QString path(variable);
        path.remove(QRegExp("^AVOGADRO_PLUGINS="));
        const QStringList searchDirs = path.split(':');
        if(!searchDirs.isEmpty())
          return searchDirs;
      }
    }

    // Check if we are running in a build directory
  #ifndef Q_WS_MAC
    if (QFile::exists(QCoreApplication::applicationDirPath()
                      + "/../CMakeCache.txt")) {
      qDebug() << "In a build directory - loading alternative...";
      return QStringList(QCoreApplication::applicationDirPath() + "/../lib");
    }
  #else
    // If we are in a Mac build dir things are a little different - if the
    // expected relative path does not exist try the build dir path
    if (QFile::exists(QCoreApplication::applicationDirPath()
                      + "/../../../../CMakeCache.txt")) {
      return QStringList(QCoreApplication::applicationDirPath()
                         + "/../../../../lib");
    }
  #endif
    else {
      QStringList searchDirs;
      // If no environment variables are set then find the plugins
      if (!searchDirs.size()) {
        // Make it relative
        searchDirs << QCoreApplication::applicationDirPath()
                       + "/../" + QString(INSTALL_LIBDIR)
                       + "/" + QString(INSTALL_PLUGIN_DIR);
      }

      // Now search for the plugins in home directories
  #if defined(Q_WS_X11)
      searchDirs << QDir::homePath() + "/."
                     + QString(INSTALL_PLUGIN_DIR) + "/plugins";
  #elif defined(Q_WS_MAC)
      searchDirs << QDir::homePath() + "/Library/Application Support/"
                     + QString(INSTALL_PLUGIN_DIR) + "/Plugins";
  #elif defined(WIN32)
      const QString appdata = qgetenv("APPDATA");
      searchDirs << appdata + "/" + QString(INSTALL_PLUGIN_DIR);
  #endif
      return searchDirs;
    }
  }

} // end namespace Avogadro

#include "mainwindow.moc"
