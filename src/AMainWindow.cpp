#include "AMainWindow.h"
#include "ASphereRender.h"
#include <fstream>

using namespace std;
using namespace OpenBabel;

AMainWindow::AMainWindow()
{
	createActions();
  createMenuBar();
  createStatusBar();
	createToolbars();

	gl = new AGLWidget(this);
	setCentralWidget(gl);

	setWindowTitle(tr("Avogadro Demo"));
}

AMainWindow::~AMainWindow()
{
}

void AMainWindow::slotOpen()
{
  QString s = QFileDialog::getOpenFileName(this);
  slotOpen(s);
}

void AMainWindow::slotOpen(QString filename)
{
  OBConversion conv;
  OBFormat     *inFormat = conv.FormatFromExt((filename.toStdString()).c_str());
  if (!inFormat || !conv.SetInFormat(inFormat))
    return;
  ifstream     ifs;
  ifs.open((filename.toStdString()).c_str());
  if (!ifs)
    return;

  view.Clear();
  if (conv.Read(&view, &ifs) && view.NumAtoms() != 0)
    {
      QString status;
      QTextStream(&status) << "Atoms: " << view.NumAtoms();
      statusBar->showMessage(status, 2000);

      ASphereRender sRender;
      gl->addDisplayList(sRender.Render(view));
    }
  else
    statusBar->showMessage("Reading molecular file failed.", 2000);
}

void AMainWindow::createActions()
{
	actionQuit = new QAction(tr("&Quit"), this);
	actionQuit->setShortcut(tr("Ctrl+Q"));
	actionQuit->setStatusTip(tr("Quit Avogadro"));
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));

  actionOpen = new QAction(tr("&Open"), this);
	actionOpen->setShortcut(tr("Ctrl+O"));
	actionOpen->setStatusTip(tr("Open"));
	connect(actionOpen, SIGNAL(triggered()), this, SLOT(slotOpen()));
}

void AMainWindow::createMenuBar()
{
  menuFile = menuBar()->addMenu(tr("&File"));

  // add actions to menus
  menuFile->addAction(actionOpen);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
}

void AMainWindow::createStatusBar()
{
  statusBar = new QStatusBar(this);
  statusBar->setObjectName(QString::fromUtf8("statusbar"));
  this->setStatusBar(statusBar);

  statusBar->showMessage(tr("Ready."), 10000);
}

void AMainWindow::createToolbars()
{
	toolBar = addToolBar(tr("File"));
  toolBar->addAction(actionOpen);
  toolBar->addAction(actionQuit);

  toolBar->setOrientation(Qt::Horizontal);
  this->addToolBar(static_cast<Qt::ToolBarArea>(4), toolBar);
}

