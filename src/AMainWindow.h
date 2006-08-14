#ifndef __AMAINWINDOW_H
#define __AMAINWINDOW_H

#include "AGLWidget.h"
#include <QMainWindow>
#include <QtGui>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

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

    QMenu      *menuFile;
    QMenu      *menuEdit;
    QMenu      *menuHelp;
    QStatusBar *statusBar;
    QToolBar   *toolBar;

    QAction    *actionQuit;
    QAction    *actionOpen;

    OpenBabel::OBMol view;

    void createActions();
    void createMenuBar();
    void createStatusBar();
    void createToolbars();
};

#endif
