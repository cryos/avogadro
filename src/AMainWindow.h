#ifndef __AMAINWINDOW_H
#define __AMAINWINDOW_H

#include "AGLWidget.h"
#include <QMainWindow>
#include <QtGui>


class AMainWindow : public QMainWindow
{
	public:
		AMainWindow();

	private:
		void createActions();
		void createToolbars();

		AGLWidget *gl;

		QAction *quitAction;
		QToolBar *fileToolBar;

};

#endif
