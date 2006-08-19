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

#include "MainWindow.h"
#include "BSRender.h"

#include <fstream>

#include <QKeySequence>
#include <QGLPixelBuffer>

using namespace std;
using namespace OpenBabel;

namespace Avogadro {

MainWindow::MainWindow()
{
  init();
  setCurrentFile("");
}

MainWindow::MainWindow(const QString &fileName)
{
  init();
  loadFile(fileName);
}

void MainWindow::init()
{
  resize(QSize(480, 320).expandedTo(this->minimumSizeHint()));
  setAttribute(Qt::WA_DeleteOnClose);
  
	createActions();
  createMenuBar();
	createToolbars();
  
  // at least for now, try to always do multisample OpenGL (i.e., antialias)
  // graphical improvement is great and many cards do this in hardware
  QGLFormat format;
  format.setSampleBuffers(true);
  gl = new GLWidget(format, this);
	setCentralWidget(gl);

  statusBar()->showMessage(tr("Ready."), 10000);
}

MainWindow::~MainWindow()
{
}

void MainWindow::newFile()
{
  MainWindow *other = new MainWindow;
  other->move(x() + 40, y() + 40);
  other->show();
}

void MainWindow::open()
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
    
    if (view.Empty())
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
    
    if (view.Empty())
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
    event->accept();
  } else {
    event->ignore();
  }
}

bool MainWindow::save()
{
  if (currentFile.isEmpty())
    return saveAs();
  else
    return saveFile(currentFile);
}

bool MainWindow::saveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this);
  if (fileName.isEmpty())
    return false;
  
  if (QFile::exists(fileName)) {
    QMessageBoxEx::StandardButton ret;
    ret = QMessageBoxEx::warning(this, tr("Avogadro"),
                                 tr("File %1 already exists.\n"
                                    "Do you want to overwrite it?")
                                 .arg(QDir::convertSeparators(fileName)),
                                 QMessageBoxEx::Yes | QMessageBoxEx::Cancel);
    if (ret == QMessageBoxEx::Cancel)
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
  if (!gl->grabFrameBuffer(true).save(fileName))
    {
      QMessageBoxEx::warning(this, tr("Avogadro"),
                             tr("Cannot save file %1.").arg(fileName));
      return;
    }
}

void MainWindow::revert()
{
  // this currently leaks -- need to free the display list and render, etc.
  view.Clear();
  loadFile(currentFile);
}

void MainWindow::undo()
{
}

void MainWindow::redo()
{
}

void MainWindow::documentWasModified()
{
  setWindowModified(true);
  isModified = false;
}

bool MainWindow::maybeSave()
{
  if (isModified) {
    QMessageBoxEx::StandardButton ret;
    ret = QMessageBoxEx::warning(this, tr("Avogadro"),
                                 tr("The document has been modified.\n"
                                    "Do you want to save your changes?"),
                                 QMessageBoxEx::Save | QMessageBoxEx::Discard
                                 | QMessageBoxEx::Cancel);
    if (ret == QMessageBoxEx::Save)
      return save();
    else if (ret == QMessageBoxEx::Cancel)
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
  QMessageBoxEx::about(this, tr("About Avogadro"),
                       tr("Avogadro is an avanced molecular editor."));
}

void MainWindow::createActions()
{
	actionQuit = new QAction(tr("&Quit"), this);
	actionQuit->setShortcut(tr("Ctrl+Q"));
	actionQuit->setStatusTip(tr("Quit Avogadro"));
	connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));

  actionNew = new QAction(tr("&New"), this);
  actionNew->setShortcut(QKeySequence::New);
  actionNew->setStatusTip(tr("Create a new file"));
  connect(actionNew, SIGNAL(triggered()), this, SLOT(newFile()));

  actionOpen = new QAction(tr("&Open..."), this);
	actionOpen->setShortcut(QKeySequence::Open);
	actionOpen->setStatusTip(tr("Open a new document"));
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));

  actionClose = new QAction(tr("&Close"), this);
	actionClose->setShortcut(QKeySequence::Close);
	actionClose->setStatusTip(tr("Close this document"));
	connect(actionClose, SIGNAL(triggered()), this, SLOT(close()));

  actionSave = new QAction(tr("&Save"), this);
  actionSave->setShortcut(QKeySequence::Save);
  actionSave->setStatusTip(tr("Save the document to disk"));
  connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
  
  actionSaveAs = new QAction(tr("Save &As..."), this);
  actionSaveAs->setShortcut(tr("Ctrl+Shift+S"));
  actionSaveAs->setStatusTip(tr("Save the document under a new name"));
  connect(actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));

  actionRevert = new QAction(tr("&Revert to Saved"), this);
	actionRevert->setStatusTip(tr("Revert to the last saved version"));
  connect(actionRevert, SIGNAL(triggered()), this, SLOT(revert()));

  actionExport = new QAction(tr("&Export Graphics..."), this);
	actionExport->setStatusTip(tr("Close this document"));
  connect(actionExport, SIGNAL(triggered()), this, SLOT(exportGraphics()));
  actionExport->setEnabled(QGLPixelBuffer::hasOpenGLPbuffers());


  for (int i = 0; i < maxRecentFiles; ++i) {
    actionRecentFile[i] = new QAction(this);
    actionRecentFile[i]->setVisible(false);
    connect(actionRecentFile[i], SIGNAL(triggered()),
            this, SLOT(openRecentFile()));
  }

  actionClearRecentMenu = new QAction(tr("&Clear Menu"), this);
  actionClearRecentMenu->setStatusTip(tr("Clear list of recent files"));
  connect(actionClearRecentMenu, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));

  actionUndo = new QAction(tr("&Undo"), this);
	actionUndo->setShortcut(QKeySequence::Undo);
	actionUndo->setStatusTip(tr("Undo change"));
 	connect(actionUndo, SIGNAL(triggered()), this, SLOT(undo()));

  actionRedo = new QAction(tr("&Redo"), this);
	actionRedo->setShortcut(QKeySequence::Redo);
	actionRedo->setStatusTip(tr("Redo change"));
  connect(actionRedo, SIGNAL(triggered()), this, SLOT(redo()));

  actionAbout = new QAction(tr("&About Avogadro"), this);
  actionAbout->setStatusTip(tr("About Avogadro"));
  connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenuBar()
{
  menuFile = menuBar()->addMenu(tr("&File"));

  // add actions to menus
  menuFile->addAction(actionNew);
  menuFile->addAction(actionOpen);
  menuOpen_Recent = menuFile->addMenu(tr("Open &Recent"));
  for (int i = 0; i < maxRecentFiles; ++i)
    menuOpen_Recent->addAction(actionRecentFile[i]);
  actionSeparator = menuOpen_Recent->addSeparator();
  menuOpen_Recent->addAction(actionClearRecentMenu);
  menuFile->addSeparator();
  menuFile->addAction(actionClose);
  menuFile->addAction(actionSave);
  menuFile->addAction(actionSaveAs);
  menuFile->addAction(actionRevert);
  menuFile->addSeparator();
  menuFile->addAction(actionExport);
  menuFile->addAction(actionQuit);

  menuEdit = menuBar()->addMenu(tr("&Edit"));
  menuEdit->addAction(actionUndo);
  menuEdit->addAction(actionRedo);

  menuHelp = menuBar()->addMenu(tr("&Help"));
  menuHelp->addAction(actionAbout);
}

void MainWindow::createToolbars()
{
	toolBar = addToolBar(tr("File"));
  toolBar->addAction(actionNew);
  toolBar->addAction(actionOpen);
  toolBar->addAction(actionClose);
  toolBar->addAction(actionSave);

  toolBar->setOrientation(Qt::Horizontal);
  this->addToolBar(static_cast<Qt::ToolBarArea>(4), toolBar);
}

bool MainWindow::loadFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBoxEx::warning(this, tr("Avogadro"),
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
    QMessageBoxEx::warning(this, tr("Avogadro"),
                           tr("Cannot read file format of file %1.")
                           .arg(fileName));
    return false;
  }
  ifstream     ifs;
  ifs.open((fileName.toStdString()).c_str());
  if (!ifs) { // shouldn't happen, already checked file above
    QMessageBoxEx::warning(this, tr("Avogadro"),
                           tr("Cannot read file %1.")
                           .arg(fileName));
    return false;
  }
  
  view.Clear();
  if (conv.Read(&view, &ifs) && view.NumAtoms() != 0)
    {
      QString status;
      QTextStream(&status) << "Atoms: " << view.NumAtoms() <<
        " Bonds: " << view.NumBonds();
      statusBar()->showMessage(status, 5000);
      
      BSRender sRender;
      gl->addDisplayList(sRender.Render(view));
      QApplication::restoreOverrideCursor();
    }
  else {
    statusBar()->showMessage("Reading molecular file failed.", 5000);
    QApplication::restoreOverrideCursor();
  }
  
  setCurrentFile(fileName);
  return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBoxEx::warning(this, tr("Avogadro"),
                           tr("Cannot write to the file %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
    return false;
  }
  
  QApplication::setOverrideCursor(Qt::WaitCursor);
  statusBar()->showMessage(tr("Saving file."), 2000);

  OBConversion conv;
  OBFormat     *outFormat = conv.FormatFromExt((fileName.toStdString()).c_str());
  if (!outFormat || !conv.SetInFormat(outFormat)) {
    QMessageBoxEx::warning(this, tr("Avogadro"),
                           tr("Cannot write to file format of file %1.")
                           .arg(fileName));
    return false;
  }
  ofstream     ofs;
  ofs.open((fileName.toStdString()).c_str());
  if (!ofs) { // shouldn't happen, already checked file above
    QMessageBoxEx::warning(this, tr("Avogadro"),
                           tr("Cannot write to the file %1.")
                           .arg(fileName));
    return false;
  }

  if (conv.Write(&view, &ofs))
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
  currentFile = fileName;
  if (currentFile.isEmpty()) {
    setWindowTitle(tr("Avogadro"));
    return;
  }
  else
    setWindowTitle(tr("%1 - %2").arg(strippedName(currentFile))
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
    actionRecentFile[i]->setText(strippedName(files[i]));
    actionRecentFile[i]->setData(files[i]);
    actionRecentFile[i]->setVisible(true);
  }
  for (int j = numRecentFiles; j < maxRecentFiles; ++j)
    actionRecentFile[j]->setVisible(false);

  actionSeparator->setVisible(numRecentFiles > 0);
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
    if (mainWin && mainWin->currentFile == canonicalFilePath)
      return mainWin;
  }
  return 0;
}

} // end namespace Avogadro

