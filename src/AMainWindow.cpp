#include "AMainWindow.h"

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
}

void AMainWindow::slotOpen(QString filename)
{
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
  menubar = new QMenuBar(this);
  menubar->setObjectName(QString::fromUtf8("menubar"));
  
  menuFile = new QMenu(menubar);
  menuFile->setObjectName(QString::fromUtf8("menuFile"));
  menubar->addAction(menuFile->menuAction());
  
  // add actions to menus
  menuFile->addAction(actionOpen);
  menuFile->addSeparator();
  menuFile->addAction(actionQuit);
  
  this->setMenuBar(menubar);
}

void AMainWindow::createStatusBar()
{
  statusBar = new QStatusBar(this);
  statusBar->setObjectName(QString::fromUtf8("statusbar"));
  this->setStatusBar(statusBar);

  statusBar->showMessage(tr("Ready."));
}

void AMainWindow::createToolbars()
{
	toolBar = addToolBar(tr("File"));
  toolBar->addAction(actionOpen);
  toolBar->addAction(actionQuit);

  toolBar->setOrientation(Qt::Horizontal);
  this->addToolBar(static_cast<Qt::ToolBarArea>(4), toolBar);
}

