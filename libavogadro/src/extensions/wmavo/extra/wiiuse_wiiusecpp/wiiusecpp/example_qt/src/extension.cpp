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


#include "extension.h"
#include "window.h"

Extension::Extension( Window *wind ) :
    m_window(wind),
    m_wm(NULL), m_wiimoteData(NULL),
    m_accXOld(0), m_accYOld(0), m_accZOld(0),
    file( "C:/Users/Myck/Desktop/out.txt" ),
    m_distanceCum(0)
{
    m_wiimoteData = new WIWO<CWiimoteData*>(SIZE_CIRCULARBUFFER) ;
    m_normalDistribution[0] = 1 ;
    m_normalDistribution[1] = 1 ;
    m_normalDistribution[2] = 1 ;
    m_normalDistribution[3] = 1 ;
    m_normalDistribution[4] = 2 ;
    m_normalDistribution[5] = 4 ;
    m_normalDistribution[6] = 8 ;
    m_normalDistribution[7] = 4 ;
    m_normalDistribution[8] = 2 ;
    m_normalDistribution[9] = 1 ;
    m_sumNormDist = 0 ;
    for( int i=0 ; i<10 ; i++ )
        m_sumNormDist += m_normalDistribution[i] ;

    connect( this, SIGNAL(xyzPositionChanged(double,double,double)),
             m_window, SLOT(setWmPosition(double,double,double)) ) ;
    connect( this, SIGNAL(xyzPositionMoyChanged(double,double,double)),
             m_window, SLOT(setWmPositionMoy(double,double,double)) ) ;
    connect( this, SIGNAL(xyzPositionGausChanged(double,double,double)),
             m_window, SLOT(setWmPositionGaus(double,double,double)) ) ;
}

Extension::~Extension()
{
    if( m_wm != NULL )
    {
        delete m_wm ;
        m_wm = NULL ;
    }

    if( m_wiimoteData != NULL )
    {
        int nbValues=m_wiimoteData->getNbCaseUse() ;
        for( int i=0 ; i<nbValues ; i++ )
            delete (*m_wiimoteData)[i] ;
        delete m_wiimoteData ;
        m_wiimoteData = NULL ;
    }
}

void Extension::connectAndStart()
{
    if( m_wm != NULL )
    { // Let to restart the connection properly.

        delete m_wm ;
        m_wm = NULL ;
    }
    m_wm = new Wiimote(&m_wmDataShare) ;

    if( m_wm->connectToDevice() )
    {
        m_wm->connect( &m_wmThread, SIGNAL(started()), SLOT(runDevice()) );
        this->connect( &m_extThread, SIGNAL(started()), SLOT(runExtension()) );

        this->moveToThread( &m_extThread ) ;
        m_wm->moveToThread( &m_wmThread ) ;

        m_extThread.start() ;
        m_wmThread.start() ;
    }
    else
    {
        delete m_wm ;
        m_wm = NULL ;
    }
}

void Extension::runExtension()
{
    while(1)
    {
        // Less, else the application crash certainly.
        #ifndef WIN32
            usleep(18000);
        #else
            //Sleep(10);
        #endif

        //if( m_paintGLdata.dataUpdated == 0 )
        //{ // Si la donnée a été traitée.

            // Update local data.
            updateData() ;

            // Update window data.
            updateWindow() ;

            // Update GLWidget.
            //updateGLWidget() ;
        //}
    }
}


void Extension::updateData()
{
    // Delete the last data of the Wiimote data stack.
    if( m_wiimoteData->isFull() )
        delete m_wiimoteData->getQueue() ;

    // Get the Wiimote data.
    m_wiimoteData->pushAtBegin( m_wmDataShare.pop() ) ;


    // Calculate average and gaussian filter.
    vect3f a, b ;
    vect3f v ;
    int nbValues=m_wiimoteData->getNbCaseUse() ;

    for( int i=0 ; i<nbValues ; i++ )
    {
        (*m_wiimoteData)[i]->Accelerometer.GetPosition(v.x, v.y, v.z) ;

        a.x += v.x ; a.y += v.y ; a.z += v.z ;
        b.x += (v.x * m_normalDistribution[i]) ;
        b.y += (v.y * m_normalDistribution[i]) ;
        b.z += (v.z * m_normalDistribution[i]) ;
    }

    m_posMoy.x=a.x/(double)nbValues; m_posMoy.y=a.y/(double)nbValues; m_posMoy.z=a.z/(double)nbValues;
    m_posGaus.x=b.x/(double)m_sumNormDist; m_posGaus.y=b.y/(double)m_sumNormDist; m_posGaus.z=b.z/(double)m_sumNormDist;

    // Round 10^10.
    //m_posMoy.x = (float)((int)(m_posMoy.x*100000000.0f)) * 0.000000001f ;
    //m_posMoy.y = (float)((int)(m_posMoy.y*100000000.0f)) * 0.000000001f ;
    //m_posMoy.z = (float)((int)(m_posMoy.z*100000000.0f)) * 0.000000001f ;
    //m_posGaus.x = (float)((int)(m_posGaus.x*100000000.0f)) * 0.000000001f ;
    //m_posGaus.y = (float)((int)(m_posGaus.y*100000000.0f)) * 0.000000001f ;
    //m_posGaus.z = (float)((int)(m_posGaus.z*100000000.0f)) * 0.000000001f ;
}

void Extension::updateWindow()
{
    if( m_wiimoteData->getNbCaseUse() > 2 )
    {
        CWiimoteData *wm0=(*m_wiimoteData)[0] ;
        CWiimoteData *wm1=(*m_wiimoteData)[1] ;
        CWiimoteData *wm2=(*m_wiimoteData)[2] ;

        bool ok = m_window->isCheckFile() ;
        double a ;
        double x,y,z=0 ;
        wm0->Accelerometer.GetTime( x, a ) ;
        wm1->Accelerometer.GetTime( y, a ) ;
        //y = wm2->Accelerometer.GetGForceElapse() ;
        //z = (wm0->Accelerometer.GetGForceInG()-1.0)*9.81 ;


        /*
        wm1->Accelerometer.GetVelocity( y, a, a);
        wm1->Accelerometer.GetGForceInMS2( z, a, a );
        double x1,y1,z1 ; wm0->Accelerometer.GetGForceInG( x1, y1, z1 ) ;
        double x2,y2,z2 ; wm0->Accelerometer.GetJerkInMS3(x2,y2,z2) ;
        double x3,y3,z3 ; wm0->Accelerometer.GetAcceleration(x3,y3,z3) ;
        double x4,y4,z4 ; wm1->Accelerometer.GetVelocity(x4,y4,z4) ;
        double x5,y5,z5 ; wm0->Accelerometer.GetPosition(x5,y5,z5) ;
        double x6,y6,z6 ; wm1->Accelerometer.GetPosition(x6,y6,z6) ;
        double x7,y7,z7 ; wm0->Accelerometer.GetTmp(x7, y7, z7);
        m_distanceCum += fabs(x5-x6) ;
        */

        double x1,y1,z1 ; wm0->IR.GetCursorDelta( x1, y1, z1 ) ;
        int x2,y2 ; wm0->IR.GetCursorPosition( x2, y2 ) ;
        double x3=(double)wm0->IR.IsInPrecisionMode() ;

        // To let to get value if one of it bug
        if( ok )
        {
            if( !file.isOpen() )
                if( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
                    QMessageBox( QMessageBox::Critical, QString("ERROR"), QString("POWNED2, File not created") ) ;

            QTextStream out( &file ) ;
            char str[150] ;
            //sprintf( str,
            //         "%+.8lf : %+.8lf : %+.8lf || %+.8lf : %+.8lf : %+.8lf || %+.8lf : %+.8lf : %+.8lf\n",
            //         x, y, z, t0, dt1, adt0, dt0dt2, v1, dt0LessDt2FoisV1 ) ;

            //sprintf( str,
            //         "%+.8lf;%+.8lf;%+.8lf;%+.8lf;%+.8lf;%+.8lf\n",
            //         x, x1, x2, x3, x4, x5 ) ;
            out << QString(str) ;
        }
        else
        {
            if( file.isOpen() )
                file.close() ;

            //emit xyzPositionsChanged( x, y, z, x7, y7, z7, x5, x5-x6, m_distanceCum ) ;
            emit xyzPositionsChanged( x, y, z, x1, y1, z1, x2, y2, x3 ) ;
        }
    }
}


void Extension::updateGLWidget()
{
    translateObjectInGLWidget() ;
}


void Extension::translateObjectInGLWidget()
{
    /*
    if( m_paintGLdata.dataUpdated == 1 )
    {
        glTranslatef(m_xTransl, m_yTransl, -m_zTransl) ;
        m_paintGLdata.dataUpdated = false ;
        m_quit.fetchAndStoreRelaxed(0) ;
    }
    */
}
