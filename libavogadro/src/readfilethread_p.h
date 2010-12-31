/**********************************************************************
  Copyright (C) 2009-2010 Marcus D. Hanwell, Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef READFILETHREAD_P_H
#define READFILETHREAD_P_H

#include <QtCore/QThread>

namespace OpenBabel {
class OBMol;
}

namespace Avogadro
{

class MoleculeFile;

class ReadFileThread : public QThread
{
  Q_OBJECT

public:
  ReadFileThread(MoleculeFile *moleculeFile);

  void addConformer(const OpenBabel::OBMol &conformer);

  void detectConformers(unsigned int c, const OpenBabel::OBMol &first,
                        const OpenBabel::OBMol &current);

  void run();

  MoleculeFile *m_moleculeFile;
};

} // End of namespace

#endif // READFILETHREAD_P_H
