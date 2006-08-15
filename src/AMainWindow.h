#ifndef __AMAINWINDOW_H
#define __AMAINWINDOW_H

#include <QMainWindow>
#include <QtGui>

#include "AGLWidget.h"

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

namespace Avogadro {

class MainWindow : public QMainWindow
{
  Q_OBJECT

	public:
		MainWindow();
    ~MainWindow();

 private slots:
    void newFile();
    void open();
    void openRecentFile();
    void save();
    void saveAs();
    void export();
    void about();

	private:
		AGLWidget  *gl;
    OpenBabel::OBMol view;
    QString    currentFile;

    QMenu      *menuFile;
    QMenu      *menuEdit;
    QMenu      *menuHelp;
    QStatusBar *statusBar;
    QToolBar   *toolBar;

    QAction    *actionQuit;

    QAction    *actionNew;
    QAction    *actionOpen;
    QAction    *actionClose;
    QAction    *actionSave;
    QAction    *actionSaveAs;
    QAction    *actionRevert;
    QAction    *actionExport;

    enum { MaxRecentFiles = 5 };
    QAction    *actionRecentFile[MaxRecentFiles];
    QAction    *actionSeparator;

    QAction    *actionAbout;
    
    void createActions();
    void createMenuBar();
    void createStatusBar();
    void createToolbars();
    void loadFile(const QString &fileName);
    void saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);
};

} // end namespace Avogadro
#endif
