#ifndef __AMAINWINDOW_H
#define __AMAINWINDOW_H

#include "AGLWidget.h"
#include <QMainWindow>
#include <QtGui>

class AMainWindow : public QMainWindow
{
  Q_OBJECT

	public:
		AMainWindow();
    ~AMainWindow();

 public slots:
    void slotOpen();
    void slotOpen(QString filename);

	private:
		AGLWidget  *gl;

    QMenuBar   *menubar;
    QMenu      *menuFile;
    QStatusBar *statusBar;
    QToolBar   *toolBar;

    QAction    *actionQuit;
    QAction    *actionOpen;

    void createActions();
    void createMenuBar();
    void createStatusBar();
    void createToolbars();
};

#endif
