/********************************************************************************
** Form generated from reading ui file 'mainwindow.ui'
**
** Created: Mon Jan 22 19:01:47 2007
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "avogadro/glwidget.h"
#include "avogadro/moleculetreeview.h"
#include "flowlayout.h"

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionQuit;
    QAction *actionRecent;
    QAction *actionClearRecent;
    QAction *actionNew;
    QAction *actionClose;
    QAction *actionSave;
    QAction *actionSaveAs;
    QAction *actionRevert;
    QAction *actionExportGraphics;
    QAction *actionFullScreen;
    QAction *actionSetBackgroundColor;
    QAction *actionAbout;
    QAction *actionUndo;
    QAction *actionRedo;
    QWidget *centralwidget;
    QVBoxLayout *vboxLayout;
    Avogadro::GLWidget *glView;
    QMenuBar *menubar;
    QMenu *menuSettings;
    QMenu *menuHelp;
    QMenu *menuView;
    QMenu *menuDocks;
    QMenu *menuToolbars;
    QMenu *menuEdit;
    QMenu *menuFile;
    QMenu *menuOpenRecent;
    QStatusBar *statusbar;
    QToolBar *tbFile;
    QDockWidget *dockProject;
    QWidget *dockProjectContents;
    QVBoxLayout *vboxLayout1;
    Avogadro::MoleculeTreeView *treeView;
    QDockWidget *dockTools;
    QWidget *dockToolsContents;
    Avogadro::FlowLayout *flowTools;
    QDockWidget *dockToolProperties;
    QWidget *dockToolPropertiesContents;

    void setupUi(QMainWindow *MainWindow)
    {
    MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    actionOpen = new QAction(MainWindow);
    actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
    actionOpen->setIcon(QIcon(QString::fromUtf8(":/icons/fileopen.png")));
    actionQuit = new QAction(MainWindow);
    actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
    actionQuit->setIcon(QIcon(QString::fromUtf8(":/icons/exit.png")));
    actionRecent = new QAction(MainWindow);
    actionRecent->setObjectName(QString::fromUtf8("actionRecent"));
    actionClearRecent = new QAction(MainWindow);
    actionClearRecent->setObjectName(QString::fromUtf8("actionClearRecent"));
    actionNew = new QAction(MainWindow);
    actionNew->setObjectName(QString::fromUtf8("actionNew"));
    actionNew->setIcon(QIcon(QString::fromUtf8(":/icons/filenew.png")));
    actionClose = new QAction(MainWindow);
    actionClose->setObjectName(QString::fromUtf8("actionClose"));
    actionClose->setIcon(QIcon(QString::fromUtf8(":/icons/fileclose.png")));
    actionSave = new QAction(MainWindow);
    actionSave->setObjectName(QString::fromUtf8("actionSave"));
    actionSave->setIcon(QIcon(QString::fromUtf8(":/icons/filesave.png")));
    actionSaveAs = new QAction(MainWindow);
    actionSaveAs->setObjectName(QString::fromUtf8("actionSaveAs"));
    actionSaveAs->setIcon(QIcon(QString::fromUtf8(":/icons/filesaveas.png")));
    actionRevert = new QAction(MainWindow);
    actionRevert->setObjectName(QString::fromUtf8("actionRevert"));
    actionRevert->setIcon(QIcon(QString::fromUtf8(":/icons/revert.png")));
    actionExportGraphics = new QAction(MainWindow);
    actionExportGraphics->setObjectName(QString::fromUtf8("actionExportGraphics"));
    actionExportGraphics->setIcon(QIcon(QString::fromUtf8(":/icons/fileexport.png")));
    actionFullScreen = new QAction(MainWindow);
    actionFullScreen->setObjectName(QString::fromUtf8("actionFullScreen"));
    actionFullScreen->setIcon(QIcon(QString::fromUtf8(":/icons/window_fullscreen.png")));
    actionSetBackgroundColor = new QAction(MainWindow);
    actionSetBackgroundColor->setObjectName(QString::fromUtf8("actionSetBackgroundColor"));
    actionSetBackgroundColor->setIcon(QIcon(QString::fromUtf8(":/icons/colorize.png")));
    actionAbout = new QAction(MainWindow);
    actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
    actionUndo = new QAction(MainWindow);
    actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
    actionUndo->setIcon(QIcon(QString::fromUtf8(":/icons/undo.png")));
    actionRedo = new QAction(MainWindow);
    actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
    actionRedo->setIcon(QIcon(QString::fromUtf8(":/icons/redo.png")));
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    vboxLayout = new QVBoxLayout(centralwidget);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    glView = new Avogadro::GLWidget(centralwidget);
    glView->setObjectName(QString::fromUtf8("glView"));

    vboxLayout->addWidget(glView);

    MainWindow->setCentralWidget(centralwidget);
    menubar = new QMenuBar(MainWindow);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menuSettings = new QMenu(menubar);
    menuSettings->setObjectName(QString::fromUtf8("menuSettings"));
    menuHelp = new QMenu(menubar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
    menuView = new QMenu(menubar);
    menuView->setObjectName(QString::fromUtf8("menuView"));
    menuDocks = new QMenu(menuView);
    menuDocks->setObjectName(QString::fromUtf8("menuDocks"));
    menuToolbars = new QMenu(menuView);
    menuToolbars->setObjectName(QString::fromUtf8("menuToolbars"));
    menuEdit = new QMenu(menubar);
    menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
    menuFile = new QMenu(menubar);
    menuFile->setObjectName(QString::fromUtf8("menuFile"));
    menuOpenRecent = new QMenu(menuFile);
    menuOpenRecent->setObjectName(QString::fromUtf8("menuOpenRecent"));
    MainWindow->setMenuBar(menubar);
    statusbar = new QStatusBar(MainWindow);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    MainWindow->setStatusBar(statusbar);
    tbFile = new QToolBar(MainWindow);
    tbFile->setObjectName(QString::fromUtf8("tbFile"));
    tbFile->setOrientation(Qt::Horizontal);
    tbFile->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    MainWindow->addToolBar(static_cast<Qt::ToolBarArea>(4), tbFile);
    dockProject = new QDockWidget(MainWindow);
    dockProject->setObjectName(QString::fromUtf8("dockProject"));
    dockProject->setMinimumSize(QSize(200, 0));
    dockProject->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::NoDockWidgetArea|Qt::RightDockWidgetArea);
    dockProjectContents = new QWidget(dockProject);
    dockProjectContents->setObjectName(QString::fromUtf8("dockProjectContents"));
    vboxLayout1 = new QVBoxLayout(dockProjectContents);
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(0);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    treeView = new Avogadro::MoleculeTreeView(dockProjectContents);
    treeView->setObjectName(QString::fromUtf8("treeView"));
    treeView->setFrameShadow(QFrame::Plain);

    vboxLayout1->addWidget(treeView);

    dockProject->setWidget(dockProjectContents);
    MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockProject);
    dockTools = new QDockWidget(MainWindow);
    dockTools->setObjectName(QString::fromUtf8("dockTools"));
    dockTools->setMinimumSize(QSize(200, 0));
    dockToolsContents = new QWidget(dockTools);
    dockToolsContents->setObjectName(QString::fromUtf8("dockToolsContents"));
    flowTools = new Avogadro::FlowLayout(dockToolsContents);
    flowTools->setSpacing(6);
    flowTools->setMargin(6);
    flowTools->setObjectName(QString::fromUtf8("flowTools"));
    dockTools->setWidget(dockToolsContents);
    MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockTools);
    dockToolProperties = new QDockWidget(MainWindow);
    dockToolProperties->setObjectName(QString::fromUtf8("dockToolProperties"));
    dockToolProperties->setMinimumSize(QSize(200, 0));
    dockToolProperties->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::NoDockWidgetArea|Qt::RightDockWidgetArea);
    dockToolPropertiesContents = new QWidget(dockToolProperties);
    dockToolPropertiesContents->setObjectName(QString::fromUtf8("dockToolPropertiesContents"));
    dockToolProperties->setWidget(dockToolPropertiesContents);
    MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockToolProperties);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuView->menuAction());
    menubar->addAction(menuSettings->menuAction());
    menubar->addAction(menuHelp->menuAction());
    menuSettings->addAction(actionSetBackgroundColor);
    menuHelp->addAction(actionAbout);
    menuView->addAction(menuDocks->menuAction());
    menuView->addAction(menuToolbars->menuAction());
    menuView->addAction(actionFullScreen);
    menuEdit->addAction(actionUndo);
    menuEdit->addAction(actionRedo);
    menuFile->addAction(actionNew);
    menuFile->addAction(actionOpen);
    menuFile->addAction(menuOpenRecent->menuAction());
    menuFile->addSeparator();
    menuFile->addAction(actionSave);
    menuFile->addAction(actionSaveAs);
    menuFile->addAction(actionRevert);
    menuFile->addAction(actionExportGraphics);
    menuFile->addSeparator();
    menuFile->addAction(actionClose);
    menuFile->addAction(actionQuit);
    menuOpenRecent->addSeparator();
    menuOpenRecent->addAction(actionClearRecent);
    tbFile->addAction(actionNew);
    tbFile->addAction(actionOpen);
    tbFile->addAction(actionSave);
    tbFile->addAction(actionQuit);

    retranslateUi(MainWindow);

    QSize size(700, 600);
    size = size.expandedTo(MainWindow->minimumSizeHint());
    MainWindow->resize(size);

    QObject::connect(actionQuit, SIGNAL(triggered()), MainWindow, SLOT(close()));

    QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Avogadro", 0, QApplication::UnicodeUTF8));
    actionOpen->setText(QApplication::translate("MainWindow", "&Open", 0, QApplication::UnicodeUTF8));
    actionOpen->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", 0, QApplication::UnicodeUTF8));
    actionQuit->setText(QApplication::translate("MainWindow", "&Quit", 0, QApplication::UnicodeUTF8));
    actionQuit->setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", 0, QApplication::UnicodeUTF8));
    actionRecent->setText(QApplication::translate("MainWindow", "Recent", 0, QApplication::UnicodeUTF8));
    actionClearRecent->setText(QApplication::translate("MainWindow", "&Clear Recent", 0, QApplication::UnicodeUTF8));
    actionNew->setText(QApplication::translate("MainWindow", "&New", 0, QApplication::UnicodeUTF8));
    actionNew->setShortcut(QApplication::translate("MainWindow", "Ctrl+N", 0, QApplication::UnicodeUTF8));
    actionClose->setText(QApplication::translate("MainWindow", "&Close", 0, QApplication::UnicodeUTF8));
    actionClose->setShortcut(QApplication::translate("MainWindow", "Ctrl+W", 0, QApplication::UnicodeUTF8));
    actionSave->setText(QApplication::translate("MainWindow", "&Save", 0, QApplication::UnicodeUTF8));
    actionSave->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
    actionSaveAs->setText(QApplication::translate("MainWindow", "Save &As...", 0, QApplication::UnicodeUTF8));
    actionSaveAs->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+S", 0, QApplication::UnicodeUTF8));
    actionRevert->setText(QApplication::translate("MainWindow", "Revert To Saved", 0, QApplication::UnicodeUTF8));
    actionExportGraphics->setText(QApplication::translate("MainWindow", "&Export Graphics", 0, QApplication::UnicodeUTF8));
    actionFullScreen->setText(QApplication::translate("MainWindow", "&Full Screen Mode", 0, QApplication::UnicodeUTF8));
    actionSetBackgroundColor->setText(QApplication::translate("MainWindow", "Set &Background Color...", 0, QApplication::UnicodeUTF8));
    actionAbout->setText(QApplication::translate("MainWindow", "&About", 0, QApplication::UnicodeUTF8));
    actionUndo->setText(QApplication::translate("MainWindow", "&Undo", 0, QApplication::UnicodeUTF8));
    actionUndo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Z", 0, QApplication::UnicodeUTF8));
    actionRedo->setText(QApplication::translate("MainWindow", "&Redo", 0, QApplication::UnicodeUTF8));
    menuSettings->setTitle(QApplication::translate("MainWindow", "&Settings", 0, QApplication::UnicodeUTF8));
    menuHelp->setTitle(QApplication::translate("MainWindow", "&Help", 0, QApplication::UnicodeUTF8));
    menuView->setTitle(QApplication::translate("MainWindow", "&View", 0, QApplication::UnicodeUTF8));
    menuDocks->setTitle(QApplication::translate("MainWindow", "&Docks", 0, QApplication::UnicodeUTF8));
    menuToolbars->setTitle(QApplication::translate("MainWindow", "Toolbars", 0, QApplication::UnicodeUTF8));
    menuEdit->setTitle(QApplication::translate("MainWindow", "&Edit", 0, QApplication::UnicodeUTF8));
    menuFile->setTitle(QApplication::translate("MainWindow", "&File", 0, QApplication::UnicodeUTF8));
    menuOpenRecent->setTitle(QApplication::translate("MainWindow", "Open &Recent", 0, QApplication::UnicodeUTF8));
    tbFile->setWindowTitle(QApplication::translate("MainWindow", "Main Toolbar", 0, QApplication::UnicodeUTF8));
    dockProject->setWindowTitle(QApplication::translate("MainWindow", "Project", 0, QApplication::UnicodeUTF8));
    dockTools->setWindowTitle(QApplication::translate("MainWindow", "Tools", 0, QApplication::UnicodeUTF8));
    dockToolProperties->setWindowTitle(QApplication::translate("MainWindow", "Tool Properties", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

#endif // UI_MAINWINDOW_H
