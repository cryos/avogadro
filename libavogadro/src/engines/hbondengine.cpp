/**********************************************************************
  HBondEngine - Hydrogen Bond Engine

  Copyright (C) 2007 by Marcus D. Hanwell
  Copyright (C) 2007,2009 by Tim Vandermeersch

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

#include "hbondengine.h"
#include <config.h>

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/painterdevice.h>
#include <avogadro/neighborlist.h>

#include <openbabel/mol.h>
#include <openbabel/obiter.h>

#include <QMessageBox>
#include <QString>
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro {

  HBondEngine::HBondEngine(QObject *parent) : Engine(parent), m_settingsWidget(0),
                                              m_width(2), m_radius(2.0), m_angle(120)
  {
  }

  Engine *HBondEngine::clone() const
  {
    HBondEngine *engine = new HBondEngine(parent());
    engine->setAlias(alias());
    engine->setWidth(m_width);
    engine->setRadius(m_radius);
    engine->setAngle(m_angle);
    engine->setEnabled(isEnabled());

    return engine;
  }

  HBondEngine::~HBondEngine()
  {
  }

  bool HBondEngine::renderOpaque(PainterDevice *pd)
  {
    Molecule *molecule = const_cast<Molecule *>(pd->molecule());
    OBMol mol = molecule->OBMol();

    pd->painter()->setColor(1.0, 1.0, 1.0);
    int stipple = 0xF0F0; // pattern for lines

    NeighborList *nbrList = new NeighborList(molecule, m_radius, 1);
    for (unsigned int i = 0; i < molecule->numAtoms(); ++i) {
      Atom *atom = molecule->atom(i);
      bool atomIsH = atom->isHydrogen() ? true : false;

      if (!atomIsH && !isHbondAcceptor(atom))
          continue;        

      QList<Atom*> nbrs = nbrList->nbrs(atom);
      foreach(Atom *nbr, nbrs) {

        double angle = 180.0;
        Atom *hydrogen, *acceptor, *donor = 0;
 
        if (atomIsH) {
          if (!isHbondDonorH(atom) || !isHbondAcceptor(nbr))
            continue;

          hydrogen = atom;
          acceptor = nbr;
          foreach (unsigned long id, atom->neighbors()) 
            donor = static_cast<Molecule*>(atom->parent())->atomById(id);
        } else {
           if (!isHbondDonorH(nbr) || !isHbondAcceptor(atom))
            continue;

          hydrogen = nbr;
          acceptor = atom;
          foreach (unsigned long id, nbr->neighbors()) 
            donor = static_cast<Molecule*>(atom->parent())->atomById(id);
        }
        
        if (donor) {
          Eigen::Vector3d ab = *donor->pos() - *hydrogen->pos();
          Eigen::Vector3d bc = *acceptor->pos() - *hydrogen->pos();
          angle = 180. * acos( ab.dot(bc) / (ab.norm() * bc.norm()) ) / M_PI;
        }
        
        if (angle < m_angle)
          continue;

        pd->painter()->drawMultiLine(*atom->pos(), *nbr->pos(), m_width, 1, stipple); 
      } // for each nbr
    } // for each atom
 
    return true;
  }

  double HBondEngine::radius(const PainterDevice *, const Primitive *) const
  {
    return 0.0;
  }

  QWidget* HBondEngine::settingsWidget()
  {
    if(!m_settingsWidget)
      {
        m_settingsWidget = new HBondSettingsWidget();
        connect(m_settingsWidget->widthSlider, SIGNAL(valueChanged(int)), this, SLOT(setWidth(int)));
        connect(m_settingsWidget->radiusSpin, SIGNAL(valueChanged(double)), this, SLOT(setRadius(double)));
        connect(m_settingsWidget->angleSpin, SIGNAL(valueChanged(double)), this, SLOT(setAngle(double)));
        connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
        m_settingsWidget->widthSlider->setValue(m_width);
        m_settingsWidget->radiusSpin->setValue(m_radius);
        m_settingsWidget->angleSpin->setValue(m_angle);
      }
    return m_settingsWidget;
  }

  void HBondEngine::setWidth(int value)
  {
    m_width = (double) value;
    emit changed();
  }

  void HBondEngine::setRadius(double value)
  {
    m_radius = value;
    emit changed();
  }

  void HBondEngine::setAngle(double value)
  {
    m_angle = value;
    emit changed();
  }


  void HBondEngine::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  void HBondEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("width", m_width);
    settings.setValue("radius", m_radius);
    settings.setValue("angle", m_angle);
  }

  void HBondEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setWidth(settings.value("width", 2.0).toDouble());
    setRadius(settings.value("radius", 2.0).toDouble());
    setAngle(settings.value("angle", 120.0).toDouble());
    if (m_settingsWidget) {
      m_settingsWidget->widthSlider->setValue(m_width);
      m_settingsWidget->radiusSpin->setValue(m_radius);
      m_settingsWidget->angleSpin->setValue(m_angle);
    }
  }

  bool HBondEngine::isHbondAcceptor(Atom *atom)
  {
    if (atom->atomicNumber() == 8 || atom->atomicNumber() == 9)
      return true;
    if (atom->atomicNumber() == 7) {
      int boSum = 0;
      foreach (unsigned long id, atom->bonds()) 
        boSum += static_cast<Molecule*>(atom->parent())->bondById(id)->order();
      if (boSum != 4)
        return true;
    }
    return false;
  }

  bool HBondEngine::isHbondDonor(Atom *atom)
  {
    switch (atom->atomicNumber()) {
      case 7:
      case 8:
      case 9:
        break;
      default:
        return false;
    }

    foreach (unsigned long id, atom->neighbors()) {
      Atom *nbr = static_cast<Molecule*>(atom->parent())->atomById(id);
      if (nbr->isHydrogen())
        return true;
    }

    return false;
  }

  bool HBondEngine::isHbondDonorH(Atom *atom)
  {
    if (!atom->isHydrogen())
      return false;

    foreach (unsigned long id, atom->neighbors()) {
      Atom *nbr = static_cast<Molecule*>(atom->parent())->atomById(id);
      if (isHbondDonor(nbr))
        return true;
    }

    return false;
  }

}

#include "hbondengine.moc"

Q_EXPORT_PLUGIN2(hbondengine, Avogadro::HBondEngineFactory)
