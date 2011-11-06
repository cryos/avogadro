
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

  This file is part of WmAvo (WiiChem project)
  WmAvo - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmAvo is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmAvo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmAvo. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#pragma once
#ifndef __VARIOUSFEATURES_H__
#define __VARIOUSFEATURES_H__

#include <QDebug>
#include <QString>
#include <iostream>
#include <sstream>
#include <string>

#if WIN32 || _WIN32
#define NOMINMAX // To avoid Eigen compilation fatal error.
// The preprocessor symbols 'min' or 'max' are defined. If you are compiling on Windows, 
// do #define NOMINMAX to prevent windows.h from defining these symbols.
#include <windows.h>
#endif


/*
  My reminder :
  The inline must be put after each method because :
  - the method definitions are not put in others .cpp, so to avoid :
    - error LNK2005: "..." already defined in moleculemanipulation.obj
  - the method are small so the copy for each class is not a problem.
*/
namespace mytoolbox
{
  ///////////////////////////////////////////////////////////////////////////////
  //// dbgMsg
  ///////////////////////////////////////////////////////////////////////////////

  inline void dbgMsg( const QString &msg )
  {
    #ifdef WIN32
    printf( "%s\n", msg.toStdString().c_str() ) ;
    #else
    qDebug() << msg ;
    #endif
    
    fflush(stdout);
  }

  inline void dbgMsg( const QList<QString> &msg )
  {
    #ifdef WIN32
    QString msgComplete ;
    foreach( QString m, msg )
      msgComplete.append( m ) ;
      
    printf( "%s\n", msgComplete.toStdString().c_str() ) ;
    #else
    qDebug() << msg ;
    #endif
    
    fflush(stdout);
  }

  inline void dbgMsg( const std::ostringstream &msg )
  {
    const char *msg_c=msg.str().c_str() ;
    #ifdef WIN32
    printf( "%s\n", msg_c ) ;
    #else
    qDebug() << msg_c ;
    #endif
    
    fflush(stdout);
  }

  inline void dbgMsg( char *msg )
  {
    #ifdef WIN32
    printf( "%s\n", msg ) ;
    #else
    qDebug() << msg ;
    #endif
    
    fflush(stdout);
  }
  
  inline void dbgMsg( const char *msg )
  {
    #ifdef WIN32
    printf( "%s\n", msg ) ;
    #else
    qDebug() << msg ;
    #endif
    
    fflush(stdout);
  }
  
  
  inline void dbgMsg( const std::string msg )
  {
    #ifdef WIN32
    printf( "%s\n", msg.c_str() ) ;
    #else
    qDebug() << msg.c_str() ;
    #endif
    
    fflush(stdout);
  }

  
  inline void dbgMsg( int msg )
  {
    #ifdef WIN32
    printf( "%d\n", msg ) ;
    #else
    qDebug() << msg ;
    #endif
    
    fflush(stdout);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //// InitializeConsoleStdIO
  ///////////////////////////////////////////////////////////////////////////////


  #if WIN32 || _WIN32
  inline void InitializeConsoleStdIO()
  {
    //::AllocConsole();
    AllocConsole();


    // si une console est rattach�e au processus, alors il existe des fichiers
    // virtuel CONIN$ et CONOUT$ qui permettent respectivement de lire
    // et d'�crire depuis / dans cette console (voir la doc de CreateFile).

    #if _MSC_VER >= 1400 // VC++ 8
    {
      // To avoid warning C4996: 'freopen' was declared deprecated
      // This function or variable may be unsafe. Consider using freopen_s instead.
      FILE *stream;
      freopen_s( &stream, "CONIN$", "r", stdin );
      freopen_s( &stream, "CONOUT$", "w", stdout );
      freopen_s( &stream, "CONOUT$", "w", stderr );
    }
    #else
    std::freopen( "CONIN$", "r", stdin );
    std::freopen( "CONOUT$", "w", stdout );
    std::freopen( "CONOUT$", "w", stderr );
    #endif

    // la ligne suivante synchronise les flux standards C++ (cin, cout, cerr...)
    std::ios_base::sync_with_stdio();

    puts( "Your message1" ) ;
  }
  #endif

}
#endif
