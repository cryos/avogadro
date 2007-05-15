/**********************************************************************
  Debug Engine - Engine for displaying debug information.

  Copyright (C) 2007 Ross Braithwaite
  Copyright (C) 2007 Shahzad Ali
  Copyright (C) 2007 James Bunt
  Copyright (C) 2007 Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation; either version 2 of the License, or 
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include "config.h"
#include "debugengine.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <QTime>
#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

bool DebugEngine::render(GLWidget *gl)
{
  QList<Primitive *> list;

  glColor3f( 1.0, 1.0, 1.0 );

  gl->painter()->beginText();
  int x = 0, y = 0;
  y += gl->painter()->drawText(x, y, "---- Debug Information ----");
  y += gl->painter()->drawText(x, y, "FPS: " + QString::number(computeFramesPerSecond(), 'g', 3));

  y += gl->painter()->drawText(x, y, "GLWidget size: "
                                       + QString::number(gl->width())
                                       + " x "
                                       + QString::number(gl->height()) );
  
  list = queue().primitiveList(Primitive::AtomType);
  y += gl->painter()->drawText(x, y, "Atoms: " + QString::number(list.size()));

  list = queue().primitiveList(Primitive::BondType);
  y += gl->painter()->drawText(x, y, "Bonds: " + QString::number(list.size()));

  gl->painter()->endText();
  
  gl->update();

  return true;
}

inline double DebugEngine::computeFramesPerSecond()
{
  static QTime time;
  static bool firstTime = true;
  static int old_time, new_time;
  static int frames;
  static double fps;
  
  if( firstTime )
  {
    time.start();
    firstTime = false;
    old_time = time.elapsed();
    frames = 0;
    fps = 0;
  }
  
  new_time = time.elapsed();
  frames++;
  
  if( new_time - old_time > 200 )
  {
    fps = 1000.0 * frames / double( new_time - old_time );
    frames = 0;
    time.restart();
    old_time = time.elapsed();
  }
  
  return fps;
}

#include "debugengine.moc"

Q_EXPORT_PLUGIN2(debugengine, DebugEngineFactory)
