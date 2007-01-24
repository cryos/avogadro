/**********************************************************************
  MainWindow - main window, menus, main actions

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

#include "mainwindow.h"
#include <avogadro/primitives.h>

#include <fstream>

#include <QKeySequence>
#include <QGLFramebufferObject>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

  MainWindow::MainWindow() : QMainWindow(0), m_currentTool(NULL), m_molecule(NULL)
  {
    constructor();
    setCurrentFile("");
  }

  MainWindow::MainWindow(const QString &fileName) : QMainWindow(0), m_currentTool(NULL), m_molecule(NULL)
  {
    constructor();
    loadFile(fileName);
  }

  void MainWindow::constructor()
  {
    ui.setupUi(this);
    
    readSettings();
    setAttribute(Qt::WA_DeleteOnClose);

    m_undo = new QUndoStack(this);
    m_molecule = new Molecule();
    m_agTools = new QActionGroup(this);
    m_flowTools = new FlowLayout(ui.dockToolsContents);
    m_flowTools->setMargin(9);
    m_stackedToolProperties = new QStackedLayout(ui.dockToolPropertiesContents);

    // at least for now, try to always do multisample OpenGL (i.e., antialias)
    // graphical improvement is great and many cards do this in hardware
    // At some point, this should be a preference
    ui.glView->setMolecule( m_molecule);
    
    ui.treeView->setAnimated(true);
    ui.treeView->setAllColumnsShowFocus(true);
    ui.treeView->setAlternatingRowColors(true);
    //ui.treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui.treeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui.treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.treeView->setMolecule(m_molecule);
    ui.treeView->header()->hide();

//dc:     toolBox = new QToolBox(this);
//dc:     toolBox->addItem(new QWidget(this), "Tools");
//dc:     toolBox->addItem(new QWidget(this), "Tool Properties");
//dc: 
//dc:     layout->addWidget(gl,0,0);
//dc:     layout->addWidget(toolBox, 0,1);

    // add all gl engines to the dropdown
    QList<Engine *> engines = ui.glView->engines();
    for(int i=0; i< engines.size(); ++i) {
      Engine *engine = engines.at(i);
//       cbEngine->insertItem(i, engine->description(), QVariant(engine));
    }

    loadTools();

    // set the default to whatever GL has selected as default on startup
//     cbEngine->setCurrentIndex(engines.indexOf(ui.glView->getDefaultEngine()));
//     cbTool->setCurrentIndex(m_tools.indexOf(m_currentTool));

//     connect(cbEngine, SIGNAL(activated(int)), ui.glView, SLOT(setDefaultEngine(int)));
//     connect(cbTool, SIGNAL(activated(int)), this, SLOT(setCurrentTool(int)));

    connectUi();

    statusBar()->showMessage(tr("Ready."), 10000);

    qDebug() << "MainWindow Initialized" << endl;
  }

  void MainWindow::newFile()
  {
    MainWindow *other = new MainWindow;
    other->move(x() + 40, y() + 40);
    other->show();
  }

  void MainWindow::openFile()
  {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
      // check to see if we already have an open window
      MainWindow *existing = findMainWindow(fileName);
      if (existing) {
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
      }

      if (m_currentFile.isEmpty())
        loadFile(fileName);
      else {
        MainWindow *other = new MainWindow;
        other->move(x() + 40, y() + 40);
        other->show();
        other->loadFile(fileName);
      }
    }
  }

  void MainWindow::openRecentFile()
  {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
      MainWindow *existing = findMainWindow(action->data().toString());
      if (existing) {
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
      }

      if (m_currentFile.isEmpty())
        loadFile(action->data().toString());
      else {
        MainWindow *other = new MainWindow;
        other->move(x() + 40, y() + 40);
        other->show();
        other->loadFile(action->data().toString());
      }
    }
  }

  void MainWindow::closeEvent(QCloseEvent *event)
  {
    if (maybeSave()) {
      writeSettings();
      event->accept();
    } else {
      event->ignore();
    }
  }

  bool MainWindow::save()
  {
    if (m_currentFile.isEmpty())
      return saveAs();
    else
      return saveFile(m_currentFile);
  }

  bool MainWindow::saveAs()
  {
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
      return false;

    if (QFile::exists(fileName)) {
      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(this, tr("Avogadro"),
          tr("File %1 already exists.\n"
            "Do you want to overwrite it?")
          .arg(QDir::convertSeparators(fileName)),
          QMessageBox::Yes | QMessageBox::Cancel);
      if (ret == QMessageBox::Cancel)
        return false;
    }
    return saveFile(fileName);
  }

  void MainWindow::exportGraphics()
  {
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
      return;

    // render it (with alpha channel)
    if (!ui.glView->grabFrameBuffer(true).save(fileName))
    {
      QMessageBox::warning(this, tr("Avogadro"),
          tr("Cannot save file %1.").arg(fileName));
      return;
    }
  }

  void MainWindow::revert()
  {
    // this currently leaks -- need to free the display list and render, etc.
    if (!m_currentFile.isEmpty()) {
      loadFile(m_currentFile);
    }
  }

  void MainWindow::documentWasModified()
  {
    setWindowModified(true);
    m_modified = false;
  }

  bool MainWindow::maybeSave()
  {
    if (m_modified) {
      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(this, tr("Avogadro"),
          tr("The document has been modified.\n"
            "Do you want to save your changes?"),
          QMessageBox::Save | QMessageBox::Discard
          | QMessageBox::Cancel);
      if (ret == QMessageBox::Save)
        return save();
      else if (ret == QMessageBox::Cancel)
        return false;
    }
    return true;
  }

  void MainWindow::clearRecentFiles()
  {
    QSettings settings; // already set up properly via main.cpp
    QStringList files;
    settings.setValue("recentFileList", files);

    updateRecentFileActions();
  }

  void MainWindow::about()
  {
    QMessageBox::about(this, tr("About Avogadro"),
        tr("Avogadro is an avanced molecular editor."));
  }

  void MainWindow::fullScreen()
  {
    if (!this->isFullScreen()) {
      ui.actionFullScreen->setText("Normal Size");
      ui.tbFile->hide();
      statusBar()->hide();
      this->showFullScreen();
    } else {
      this->showNormal();
      ui.actionFullScreen->setText("Full Screen");
      ui.tbFile->show();
      statusBar()->show();
    }
  }

  void MainWindow::setBackgroundColor()
  {
    QColor current = ui.glView->background();
    ui.glView->setBackground(QColorDialog::getRgba(current.rgba(), NULL, this));
  }

  void MainWindow::connectUi()
  {
    connect(ui.actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(newFile()));
    connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(ui.actionClose, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(ui.actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui.actionRevert, SIGNAL(triggered()), this, SLOT(revert()));
    connect(ui.actionExportGraphics, SIGNAL(triggered()), this, SLOT(exportGraphics()));
    ui.actionExportGraphics->setEnabled(QGLFramebufferObject::hasOpenGLFramebufferObjects());

    for (int i = 0; i < maxRecentFiles; ++i) {
      m_actionRecentFile[i] = new QAction(this);
      m_actionRecentFile[i]->setVisible(false);
      ui.menuOpenRecent->addAction(m_actionRecentFile[i]);
      connect(m_actionRecentFile[i], SIGNAL(triggered()),
          this, SLOT(openRecentFile()));
    }
    
    ui.menuDocks->addAction(ui.dockProject->toggleViewAction());
    ui.menuDocks->addAction(ui.dockTools->toggleViewAction());
    ui.menuDocks->addAction(ui.dockToolProperties->toggleViewAction());
    ui.menuToolbars->addAction(ui.tbFile->toggleViewAction());
    

    connect(ui.actionClearRecent, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
    connect(ui.actionFullScreen, SIGNAL(triggered()), this, SLOT(fullScreen()));
    connect(ui.actionSetBackgroundColor, SIGNAL(triggered()), this, SLOT(setBackgroundColor()));
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    
    connect(ui.glView, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(glMousePress(QMouseEvent *)));
    connect(ui.glView, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(glMouseMove(QMouseEvent *)));
    connect(ui.glView, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(glMouseRelease(QMouseEvent *)));
    
    connect(m_agTools, SIGNAL(triggered(QAction *)), this, SLOT(setCurrentTool(QAction *)));
  }

/*  void MainWindow::createDocks()
  {
    dockTools = new QDockWidget(tr("Tools"), this);
    dockTools->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockTools->setMinimumWidth(_MW_MIN_DOCK_WIDTH);
    dockTools->setWidget(new QWidget());
    addDockWidget(Qt::LeftDockWidgetArea, dockTools);
    menuSettingsDocks->addAction(dockTools->toggleViewAction());

    dockToolProperties = new QDockWidget(tr("Tool Properties"), this);
    dockToolProperties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockToolProperties->setMinimumWidth(_MW_MIN_DOCK_WIDTH);
    dockToolProperties->setWidget(new QWidget());
    addDockWidget(Qt::LeftDockWidgetArea, dockToolProperties);
    menuSettingsDocks->addAction(dockToolProperties->toggleViewAction());

    dockProject = new QDockWidget(tr("Project"), this);
    dockProject->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockProject->setMinimumWidth(_MW_MIN_DOCK_WIDTH);
//dc:     ui.treeView = new MoleculeTreeView();
//dc:     treeProject->setAlternatingRowColors(true);
//dc:     treeProject->setAnimated(true);
//dc:     treeProject->header()->hide();
//dc:     treeProject->setAllColumnsShowFocus(true);
//dc:     treeProject->setAlternatingRowColors(true);
//dc:     treeProject->setSelectionMode(QAbstractItemView::ExtendedSelection);
//dc:     treeProject->setSelectionBehavior(QAbstractItemView::SelectRows);
//dc:     treeProject->setRootIsDecorated(false);
//dc:     dockProject->setWidget(treeProject);
    addDockWidget(Qt::RightDockWidgetArea, dockProject);
    menuSettingsDocks->addAction(dockProject->toggleViewAction());

  }
*/
  
  bool MainWindow::loadFile(const QString &fileName)
  {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Avogadro"),
          tr("Cannot read file %1:\n%2.")
          .arg(fileName)
          .arg(file.errorString()));
      return false;
    }
    file.close();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    statusBar()->showMessage(tr("Reading file."), 2000);
    OBConversion conv;
    OBFormat     *inFormat = conv.FormatFromExt((fileName.toStdString()).c_str());
    if (!inFormat || !conv.SetInFormat(inFormat)) {
      QMessageBox::warning(this, tr("Avogadro"),
          tr("Cannot read file format of file %1.")
          .arg(fileName));
      return false;
    }
    ifstream     ifs;
    ifs.open((fileName.toStdString()).c_str());
    if (!ifs) { // shouldn't happen, already checked file above
      QMessageBox::warning(this, tr("Avogadro"),
          tr("Cannot read file %1.")
          .arg(fileName));
      return false;
    }

    Molecule *molecule = new Molecule;
    if (conv.Read(molecule, &ifs) && molecule->NumAtoms() != 0)
    {
      ui.treeView->setMolecule(molecule);
      ui.glView->setMolecule(molecule);
      if(m_molecule)
        delete(m_molecule);
      m_molecule = molecule;

      QApplication::restoreOverrideCursor();

      QString status;
      QTextStream(&status) << "Atoms: " << m_molecule->NumAtoms() <<
        " Bonds: " << m_molecule->NumBonds();
      statusBar()->showMessage(status, 5000);

      
    }
    else {
      statusBar()->showMessage("Reading molecular file failed.", 5000);
      QApplication::restoreOverrideCursor();
      return false;
    }

    setCurrentFile(fileName);

    return true;
  }

  bool MainWindow::saveFile(const QString &fileName)
  {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Avogadro"),
          tr("Cannot write to the file %1:\n%2.")
          .arg(fileName)
          .arg(file.errorString()));
      return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    statusBar()->showMessage(tr("Saving file."), 2000);

    OBConversion conv;
    OBFormat     *outFormat = conv.FormatFromExt((fileName.toStdString()).c_str());
    if (!outFormat || !conv.SetOutFormat(outFormat)) {
      QMessageBox::warning(this, tr("Avogadro"),
          tr("Cannot write to file format of file %1.")
          .arg(fileName));
      return false;
    }
    ofstream     ofs;
    ofs.open((fileName.toStdString()).c_str());
    if (!ofs) { // shouldn't happen, already checked file above
      QMessageBox::warning(this, tr("Avogadro"),
          tr("Cannot write to the file %1.")
          .arg(fileName));
      return false;
    }

    OBMol *molecule = dynamic_cast<OBMol*>(m_molecule);
    if (conv.Write(molecule, &ofs))
      statusBar()->showMessage("Save succeeded.", 5000);
    else
      statusBar()->showMessage("Saving molecular file failed.", 5000);
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
  }

  void MainWindow::setCurrentFile(const QString &fileName)
  {
    m_currentFile = fileName;
    if (m_currentFile.isEmpty()) {
      setWindowTitle(tr("Avogadro"));
      return;
    }
    else
      setWindowTitle(tr("%1 - %2").arg(strippedName(m_currentFile))
          .arg(tr("Avogadro")));

    QSettings settings; // already set up properly via main.cpp
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > maxRecentFiles)
      files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
      MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
      if (mainWin)
        mainWin->updateRecentFileActions();
    }
  }

  void MainWindow::updateRecentFileActions()
  {
    QSettings settings; // set up project and program properly in main.cpp
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)maxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
      m_actionRecentFile[i]->setText(strippedName(files[i]));
      m_actionRecentFile[i]->setData(files[i]);
      m_actionRecentFile[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < maxRecentFiles; ++j)
      m_actionRecentFile[j]->setVisible(false);

//     ui.actionSeparator->setVisible(numRecentFiles > 0);
  }

  QString MainWindow::strippedName(const QString &fullFileName)
  {
    return QFileInfo(fullFileName).fileName();
  }

  MainWindow *MainWindow::findMainWindow(const QString &fileName)
  {
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QWidget *widget, qApp->topLevelWidgets()) {
      MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
      if (mainWin && mainWin->m_currentFile == canonicalFilePath)
        return mainWin;
    }
    return 0;
  }

  void MainWindow::readSettings()
  {
    QSettings settings;
    //QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(640, 480)).toSize();
    resize(size);
    //move(pos);
  }

  void MainWindow::writeSettings()
  {
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
  }
    
  void MainWindow::loadTools()
  {
    QDir pluginsDir("/usr/local/lib/avogadro");

    if(getenv("AVOGADRO_TOOLS") != NULL)
    {
      pluginsDir.cd(getenv("AVOGADRO_TOOLS"));
    }

    //dc:  if (!pluginsDir.cd("m_tools") && getenv("AVOGADRO_TOOLS") != NULL)
    //dc:  {
    //dc:    pluginsDir.cd(getenv("AVOGADRO_TOOLS"));
    //dc:  }

    qDebug() << "AVOGADRO_TOOLS:" << pluginsDir.absolutePath() << endl;
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
      QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
      //qDebug() << "File: " << fileName;
      Tool *tool = qobject_cast<Tool *>(loader.instance());
      if (tool) {
        qDebug() << "Found Tool: " << tool->name() << " - " << tool->description(); 
        m_tools.append(tool);

        QAction *action = tool->selectAction();
        QToolButton *button = new QToolButton();
        button->setDefaultAction(action);
        m_agTools->addAction(action);

        m_flowTools->addWidget(button);
        m_stackedToolProperties->addWidget(tool->propertiesWidget());
        
        if (!m_currentTool)
        {
          setCurrentTool(tool);
          button->click();
        }
      
      }
    }

  }

  void MainWindow::setCurrentTool(int i)
  {
    m_stackedToolProperties->setCurrentIndex(i);
    setCurrentTool(m_tools.at(i));
  }

  void MainWindow::setCurrentTool(QAction *action)
  {
    qDebug() << m_flowTools->sizeHint();
    qDebug() << m_stackedToolProperties->sizeHint();
    Tool *tool = action->data().value<Tool *>();
    setCurrentTool(tool);
  }

  void MainWindow::setCurrentTool(Tool *tool)
  {
    m_stackedToolProperties->setCurrentWidget(tool->propertiesWidget());
    m_currentTool = tool;
  }

  void MainWindow::glMousePress(QMouseEvent *event)
  {
    if(m_currentTool)
      m_currentTool->mousePress(m_molecule, ui.glView, event);
  }

  void MainWindow::glMouseMove(QMouseEvent *event)
  {
    if(m_currentTool)
      m_currentTool->mouseMove(m_molecule, ui.glView, event);
  }

  void MainWindow::glMouseRelease(QMouseEvent *event)
  {
    if(m_currentTool)
      m_currentTool->mouseRelease(m_molecule, ui.glView, event);
  }

} // end namespace Avogadro
