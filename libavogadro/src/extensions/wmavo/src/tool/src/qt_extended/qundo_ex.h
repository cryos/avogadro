
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
#ifndef __QUNDO_EX_H__
#define __QUNDO_EX_H__

#include "warning_disable_begin.h"
#include <QUndoCommand>
#include "warning_disable_end.h"

// Avoir une donnée de retour dans le constructeur
// + http://lynxline.com/undo-in-complex-qt-projects/
// + des pointeurs sur fonction en paramètre pour éviter d'avoir des classes 
//    en doublon (pour les méthodes avec les mêmes prototypes).
 class QUndoCommand_ex : public QUndoCommand
 {
 public:
     QUndoCommand_ex( Molecule *molecule, void* donneeDeRetour ){};
     void undo(){};
     void redo(){};

     void *getDonneeDeRetour(){} ;

 private :
   void *donneeDeRetour ;
 };

#endif
