/**********************************************************************
  BasisSet - Base class for basis sets

  Copyright (C) David C. Lonie
  Copyright (C) 2008-2009 Marcus D. Hanwell
  Copyright (C) 2008 Albert De Fusco

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef BASISSET_H
#define BASISSET_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QList>

#include <Eigen/Core>
#include <vector>

namespace Avogadro
{
  class Molecule;
  class Cube;

  class BasisSet : public QObject
  {
  Q_OBJECT

  public:
    /**
     * Constructor.
     */
    BasisSet() : m_electrons(0) {};

    /**
     * Destructor.
     */
    virtual ~BasisSet() {};

    /**
     * Set the number of electrons in the BasisSet.
     * @param n The number of electrons in the BasisSet.
     */
    void setNumElectrons(unsigned int n) { m_electrons = n; }

    /**
     * @return The number of electrons in the molecule.
     */
    unsigned int numElectrons() { return m_electrons; }

    /**
     * @return The number of MOs in the BasisSet.
     */
    virtual unsigned int numMOs()=0;

    /**
     * Check if the given MO number is the HOMO or not.
     * @param n The MO number.
     * @return True if the given MO number is the HOMO.
     */
    bool HOMO(unsigned int n)
    {
      if (n+1 == static_cast<unsigned int>(m_electrons / 2))
        return true;
      else
        return false;
    }

    /**
     * Check if the given MO number is the LUMO or not.
     * @param n The MO number.
     * @return True if the given MO number is the LUMO.
     */
    bool LUMO(unsigned int n)
    {
      if (n == static_cast<unsigned int>(m_electrons / 2))
        return true;
      else
        return false;
    }

    /**
     * Calculate the MO over the entire range of the supplied Cube.
     * @param cube The cube to write the values of the MO into.
     * @return True if the calculation was successful.
     */
    virtual bool calculateCubeMO(Cube *cube, unsigned int state = 1)=0;

    /**
     * Calculate the electron density over the entire range of the supplied Cube.
     * @param cube The cube to write the values of the MO into.
     * @return True if the calculation was successful.
     */
    virtual bool calculateCubeDensity(Cube *cube)=0;

    /**
     * When performing a calculation the QFutureWatcher is useful if you want
     * to update a progress bar.
     */
    virtual QFutureWatcher<void> & watcher()=0;

    /**
     * Basis Set Type. Override with subclass methods.
     */
    virtual QString basisSetType() const { return QString("Unknown"); };
    virtual void expandIntoPrimitives() { return; };
    virtual QList<qreal> X0List() const { QList<qreal> value; return value; };
    virtual QList<qreal> Y0List() const { QList<qreal> value; return value; };
    virtual QList<qreal> Z0List() const { QList<qreal> value; return value; };
    virtual QList<qint64> xamomList() const { QList<qint64> value; return value; };
    virtual QList<qint64> yamomList() const { QList<qint64> value; return value; };
    virtual QList<qint64> zamomList() const { QList<qint64> value; return value; };
    virtual QList<qreal> alphaList() const { QList<qreal> value; return value; };
    virtual QList<qreal> orbeList() const { QList<qreal> value; return value; };
    virtual QList<qreal> occnoList() const { QList<qreal> value; return value; };
    virtual QList<qreal> coefList() const { QList<qreal> value; return value; };

  protected:
    /// Total number of electrons
    unsigned int m_electrons;

  };

} // End namespace Avogadro

#endif
