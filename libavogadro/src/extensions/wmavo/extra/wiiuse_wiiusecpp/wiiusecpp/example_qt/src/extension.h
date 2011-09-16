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

#ifndef _EXTENSION_H_
#define _EXTENSION_H_

#include "wiimote.h"
#include "wiimotedata_share.h"
#include "qthread_ex.h"
#include "wiwo.h"

//#include <QGLWidget>
#include <QAtomicInt>
#include <QObject>
#include <QFile>

#include <iostream>
#include <iomanip>

class Window ;

class Extension : public QObject
{
    Q_OBJECT

// Public methods.
public :
    Extension( Window *wind ) ;
    ~Extension() ;

public Q_SLOTS :
    /**
      * Connect & start.
      */
    void connectAndStart() ;

    /**
      * Dedicated to the wiimote class.
      * @{ */
    void runExtension() ;
    // @}


signals :

    void xyzPositionChanged( double x, double y, double z ) ;
    void xyzPositionMoyChanged( double x, double y, double z ) ;
    void xyzPositionGausChanged( double x, double y, double z ) ;
    void xyzPositionsChanged( double x, double y, double z, double x1, double y1, double z1, double x2, double y2, double z2 ) ;

public :

    // @}

    /**
      * Dedicated to this.
      */
    void updateData() ;

    /**
      * Dedicated to the window/glwidget class.
      * @{ */
    void updateWindow() ;
    void updateGLWidget() ;
    void translateObjectInGLWidget() ;
    // @}


// Data defintion.
public :
    // Structure spécifique aux données traitées par l'application.
    /*
    struct paintGLData
    {
        QAtomicInt dataUpdated ;
        float xTransl, yTransl, zTransl ;

        paintGLData(): dataUpdated(false), xTransl(0), yTransl(0), zTransl(0) {};
    };
    */

//Private attributs.
private :
    Window *m_window ; //< Shortcut.
    //QGLWidget *m_widget ; //< Shortcut.
    //paintGLData m_paintGLdata ;

    Wiimote *m_wm ; //< Object : All method to manipulate the Wiimote.
    WIWO<CWiimoteData*> *m_wiimoteData ; //< Object : a copy of the Wiimote data.
    WiimotedataShare m_wmDataShare ; //< The mecanism to get the data of the Wiimote.

    QThread_ex m_extThread, m_wmThread ;

    struct vect3f
    {
        double x;double y;double z;
        vect3f(){x=0;y=0;z=0;}
    } ;

    vect3f m_posMoy, m_posGaus ;
    float m_normalDistribution[10] ;
    int m_sumNormDist ;
    float m_accXOld, m_accYOld, m_accZOld ;
    double m_distanceCum ;

public :
    QFile file ;
};

#endif
