#include "AMainWindow.h"

AMainWindow::AMainWindow()
{

	gl = new AGLWidget(this);
	setCentralWidget(gl);

	createActions();
	createToolbars();
	setWindowTitle(tr("Avogadro Demo"));


}

void AMainWindow::createActions()
{
	quitAction = new QAction(tr("&Quit"), this);
	quitAction->setShortcut(tr("Ctrl+Q"));
	quitAction->setStatusTip(tr("Quit Avogadro"));
	connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
}

void AMainWindow::createToolbars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(quitAction);
}

