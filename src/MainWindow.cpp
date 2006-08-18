#include "MainWindow.h"
#include "ASphereRender.h"
#include "ABSRender.h"
#include "AStickRender.h"
#include <fstream>

#include <QKeySequence>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;

MainWindow::MainWindow()
{
  resize(QSize(480, 320).expandedTo(this->minimumSizeHint()));
  setAttribute(Qt::WA_DeleteOnClose);

	createActions();
  createMenuBar();
  createStatusBar();
	createToolbars();
  
  // at least for now, try to always do multisample OpenGL (i.e., antialias)
  QGLFormat format;
  format.setSampleBuffers(true);
	gl = new AGLWidget(format, this);
	setCentralWidget(gl);

  setWindowTitle(tr("Avogadro"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::newFile()
{
  MainWindow *other = new MainWindow;
  other->show();
}

void MainWindow::open()
{
  QString fileName = QFileDialog::getOpenFileName(this);
  if (!filename.isEmpty())
    loadFile(fileName);
}

void MainWindow::loadFile(QString fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBoxEx::warning(this, tr("Avogadro"),
                           tr("Cannot read file %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
    return;
  }
  file.close();

  OBConversion conv;
  OBFormat     *inFormat = conv.FormatFromExt((fileName.toStdString()).c_str());
  if (!inFormat || !conv.SetInFormat(inFormat)) {
    QMessageBoxEx::warning(this, tr("Avogadro"),
                           tr("Cannot read file format of file %1.")
                           .arg(fileName));
    return;
  }
  ifstream     ifs;
  ifs.open((fileName.toStdString()).c_str());
  if (!ifs) { // shouldn't happen, already checked file above
    QMessageBoxEx::warning(this, tr("Avogadro"),
                           tr("Cannot read file %1.")
                           .arg(fileName));
    return;
  }

  view.Clear();
  QApplication::setOverrideCursor(Qt::WaitCursor);
  if (conv.Read(&view, &ifs) && view.NumAtoms() != 0)
    {
      QString status;
      QTextStream(&status) << "Atoms: " << view.NumAtoms() <<
        " Bonds: " << view.NumBonds();
      statusBar->showMessage(status, 2000);

      AStickRender sRender;
      gl->addDisplayList(sRender.Render(view));
    }
  else
    statusBar->showMessage("Reading molecular file failed.", 2000);
  QApplication::restoreOverrideCursor();

  setCurrentFile(fileName);
}

void MainWindow::save()
{
  if (curFile.isEmpty())
    saveAs();
  else
    saveFile(curFile);
}

void MainWindow::saveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this);
  if (fileName.isEmpty())
    return;
  
  if (QFile::exists(fileName)) {
    QMessageBoxEx::StandardButton ret;
    ret = QMessageBoxEx::warning(this, tr("Recent Files"),
                                 tr("File %1 already exists.\n"
                                    "Do you want to overwrite it?")
                         .arg(QDir::convertSeparators(fileName)),
                                 QMessageBoxEx::Yes | QMessageBoxEx::Cancel);
    if (ret == QMessageBoxEx::Cancel)
      return;
  }
  saveFile(fileName);
}



void MainWindow::createActions()
{
	actionQuit = new QAction(tr("&Quit"), this);
	actionQuit->setShortcut(tr("Ctrl+Q"));
	actionQuit->setStatusTip(tr("Quit Avogadro"));
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(closeAllWindows()));

  actionNew = new QAction(tr("&New"), this);
  actionNew->setShortcut(QKeySequence::New);
  actionNew->setStatusTip(tr("Create a new file"));
  connect(actionNew, SIGNAL(triggered()), this, SLOT(newFile()));

  actionOpen = new QAction(tr("&Open"), this);
	actionOpen->setShortcut(QKeySequence::Open);
	actionOpen->setStatusTip(tr("Open..."));
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));

  actionAbout = new QAction(tr("&About"), this);
  actionAbout->setStatusTip(tr("About Avogadro"));
  connect(actionAbout, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenuBar()
{
  menuFile = menuBar()->addMenu(tr("&File"));

  // add actions to menus
  menuFile->addAction(actionNew);
  menuFile->addAction(actionOpen);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
}

void MainWindow::createStatusBar()
{
  statusBar = new QStatusBar(this);
  statusBar->setObjectName(QString::fromUtf8("statusbar"));
  this->setStatusBar(statusBar);

  statusBar->showMessage(tr("Ready."), 10000);
}

void MainWindow::createToolbars()
{
	toolBar = addToolBar(tr("File"));
  toolBar->addAction(actionOpen);
  toolBar->addAction(actionQuit);

  toolBar->setOrientation(Qt::Horizontal);
  this->addToolBar(static_cast<Qt::ToolBarArea>(4), toolBar);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

void MainWindow::about()
{
  QMessageBoxEx::about(this, tr("About Avogadro"),
                       tr("Avogadro is an avanced molecular editor"));
}
