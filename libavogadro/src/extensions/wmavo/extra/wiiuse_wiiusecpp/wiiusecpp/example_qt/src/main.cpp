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


#include <QApplication>
#include <QDesktopWidget>

#include "window.h"
#include "qthread_ex.h"
#include "wiimote.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv) ;
    Window window ;
    int desktopArea=0, widgetArea=0 ;

    window.resize(window.sizeHint()) ;
    widgetArea = window.width() * window.height() ;
    desktopArea = QApplication::desktop()->width()
                  * QApplication::desktop()->height() ;

    if( ((float)widgetArea / (float)desktopArea) < 0.75f )
        window.show() ;
    else
        window.showMaximized() ;

    return app.exec() ;
}
