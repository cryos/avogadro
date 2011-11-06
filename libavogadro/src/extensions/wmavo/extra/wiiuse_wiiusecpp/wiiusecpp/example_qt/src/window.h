/*
 *      Copyright (c) 2011 Mickael Gadroy
 *
 *	This file is part of example_qt of the wiiusecpp library.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef WINDOW_H
#define WINDOW_H

//#define SLIDER_ROT_CONST 16

#include <QWidget>
#include <QtGui>
#include <QFile>

#include "glwidget.h"
#include "extension.h"

class QSlider;
class GLWidget;

 class Window : public QWidget
 {
     Q_OBJECT

 public:
     Window() ;
     ~Window() ;

     bool isCheckFile() ;

 public slots :
     void setWmPosition( double x, double y, double z ) ;
     void setWmPositionMoy( double x, double y, double z ) ;
     void setWmPositionGaus( double x, double y, double z ) ;
     void setWmTable( double x, double y, double z, double x1, double y1, double z1, double x2, double y2, double z2 ) ;

 signals :
     void xyzTranslationChanged( float x, float y, float z ) ;

 protected:
     void keyPressEvent(QKeyEvent *event) ;

 private:
     QSlider* createSliderRotation() ;
     QSlider* createSliderTranslation() ;
     QLineEdit* createTextBox() ;
     QAbstractItemModel* createTable( QSplitter &splitter_out ) ;

 private:
     GLWidget *m_glWidget ;
     Extension *m_extension ;

     QSlider *m_sliderXRot, *m_sliderYRot, *m_sliderZRot ;
     QSlider *m_sliderXTransl, *m_sliderYTransl, *m_sliderZTransl ;

     QPushButton *m_buttonStartWm ;

     QLabel *m_labelPos, *m_labelPosX, *m_labelPosY, *m_labelPosZ ;
     QLineEdit *m_textBoxPosX, *m_textBoxPosY, *m_textBoxPosZ ;

     QLabel *m_labelPosMoy, *m_labelPosXMoy, *m_labelPosYMoy, *m_labelPosZMoy ;
     QLineEdit *m_textBoxPosXMoy, *m_textBoxPosYMoy, *m_textBoxPosZMoy ;

     QLabel *m_labelPosGaus, *m_labelPosXGaus, *m_labelPosYGaus, *m_labelPosZGaus ;
     QLineEdit *m_textBoxPosXGaus, *m_textBoxPosYGaus, *m_textBoxPosZGaus ;

     QAbstractItemModel *m_modelTable ; // QTableView ...
     int m_rowCurrent ;

     QCheckBox *m_checkBoxActivated, *m_checkBoxFile, *m_checkBoxTable ;

 public :
     QFile file ;
 };

 #endif

