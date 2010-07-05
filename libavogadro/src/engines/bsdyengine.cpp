/**********************************************************************
  BSDYEngine - Dynamic detail engine for "balls and sticks" display

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include "bsdyengine.h"

#include <avogadro/camera.h>
#include <avogadro/painter.h>
#include <avogadro/painterdevice.h>
#include <avogadro/color.h>

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

#include <QGLWidget> // for OpenGL bits
#include <QDebug>

#include <openbabel/mol.h>

using namespace std;
using namespace Eigen;

namespace Avogadro
{

// our sort function
/*  Camera *camera = 0;
  bool sortCameraFarthest( const Primitive* lhs, const Primitive* rhs )
  {
    if ( !lhs ) {
      if ( rhs ) {
        return true;
      } else {
        return false;
      }
    }

    if ( lhs->type() == Primitive::BondType && rhs->type() == Primitive::BondType ) {
      if ( camera ) {
        const Bond *l = static_cast<const Bond *>( lhs );
        const Bond *r = static_cast<const Bond *>( rhs );

        const Atom* latom1 = static_cast<const Atom *>( l->GetBeginAtom() );
        const Atom* latom2 = static_cast<const Atom *>( l->GetEndAtom() );
        Vector3d lv1( latom1->pos() );
        Vector3d lv2( latom2->pos() );
        Vector3d ld1 = lv2 - lv1;
        ld1.normalize();

        const Atom* ratom1 = static_cast<const Atom *>( r->GetBeginAtom() );
        const Atom* ratom2 = static_cast<const Atom *>( r->GetEndAtom() );
        Vector3d rv1( ratom1->pos() );
        Vector3d rv2( ratom2->pos() );
        Vector3d rd1 = rv2 - rv1;
        return camera->distance( ld1 ) >= camera->distance( rd1 );
      }
    } else if ( lhs->type() == Primitive::AtomType && rhs->type() == Primitive::AtomType ) {
      if ( camera ) {
        const Atom *l = static_cast<const Atom*>( lhs );
        const Atom *r = static_cast<const Atom*>( rhs );
        return camera->distance( l->pos() ) >= camera->distance( r->pos() );
      }
    }
    return false;
  } */

  BSDYEngine::BSDYEngine(QObject *parent) : Engine(parent),
      m_settingsWidget(0), m_atomRadiusPercentage(0.3), m_bondRadius(0.1),
      m_atomRadiusType(1), m_showMulti(2), m_alpha(1.)
  {  }

  Engine *BSDYEngine::clone() const
  {
    BSDYEngine *engine = new BSDYEngine(parent());
    engine->setAlias(alias());
    engine->m_atomRadiusPercentage = m_atomRadiusPercentage;
    engine->m_bondRadius = m_bondRadius;
    engine->m_showMulti = m_showMulti;
    engine->m_atomRadiusType = m_atomRadiusType;
    engine->m_alpha = m_alpha;
    engine->setEnabled(isEnabled());

    return engine;
  }

  BSDYEngine::~BSDYEngine()
  {
    if ( m_settingsWidget ) {
      m_settingsWidget->deleteLater();
    }
  }

  bool BSDYEngine::renderOpaque( PainterDevice *pd )
  {
//    glPushAttrib( GL_TRANSFORM_BIT );

    // Render the opaque balls & sticks if m_alpha is 1
    if (m_alpha < 0.999) {
      return true;
    }
    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    // Render the bonds
    foreach(const Bond *b, bonds()) {
      Atom* atom1 = pd->molecule()->atomById(b->beginAtomId());
      Atom* atom2 = pd->molecule()->atomById(b->endAtomId());
      if (!atom1 || !atom2) {
        qDebug() << "Invalid bond atom IDs" << b->beginAtomId() << atom1
                 << b->endAtomId() << atom2 << "Bond" << b->id();
        continue;
      }

      Vector3d v1(*atom1->pos());
      Vector3d v2(*atom2->pos());
      Vector3d d = v2 - v1;
      d.normalize();
      Vector3d v3((v1 + v2 + d*(radius(atom1) - radius(atom2))) / 2);

      double shift = 0.15;
      int order = 1;
      if (m_showMulti) order = b->order();

      map->setFromPrimitive(atom1);
      if (atom1->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom1->customColorName());
      pd->painter()->drawMultiCylinder( v1, v3, m_bondRadius, order, shift );

      map->setFromPrimitive(atom2);
      if (atom2->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(atom2->customColorName());
      pd->painter()->drawMultiCylinder( v3, v2, m_bondRadius, order, shift );
    }

    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );

    // Render the atoms
    foreach(const Atom *a, atoms()) {
      map->setFromPrimitive(a);
      if (a->customColorName().isEmpty())
        pd->painter()->setColor( map );
      else
        pd->painter()->setColor(a->customColorName());
      pd->painter()->drawSphere(a->pos(), radius(a));
    }

    // normalize normal vectors of bonds
    glDisable( GL_RESCALE_NORMAL );
    glEnable( GL_NORMALIZE );

//    glPopAttrib();

    return true;
  }

  bool BSDYEngine::renderTransparent(PainterDevice *pd)
  {
    // Render selections when not renderquick
    Color *map = colorMap();
    if (!map) map = pd->colorMap();

    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );
    foreach(const Atom *a, atoms()) {
      // First render the atom if it is transparent.
      if (m_alpha < 0.999 && m_alpha > 0.001) {
        map->setFromPrimitive(a);
        map->setAlpha(m_alpha);
        pd->painter()->setColor(map);
        pd->painter()->drawSphere(a->pos(), radius(a));
      }
      // If the atom is selected render the selection
      if (pd->isSelected(a)) {
        map->setToSelectionColor();
        pd->painter()->setColor(map);
        pd->painter()->drawSphere(a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a));
      }
    }

    glDisable( GL_RESCALE_NORMAL );
    glEnable( GL_NORMALIZE );
    foreach(const Bond *b, bonds()) {
      // If the bond is not selected and balls and sticks are opaque do not render it
      if (!pd->isSelected(b) && m_alpha > 0.999) continue;

      Atom* atom1 = pd->molecule()->atomById(b->beginAtomId());
      Atom* atom2 = pd->molecule()->atomById(b->endAtomId());
      if (!atom1 || !atom2) {
        qDebug() << "Invalid bond atom IDs" << b->beginAtomId() << atom1
                 << b->endAtomId() << atom2 << "Bond" << b->id();
        continue;
      }

      Vector3d v1(*atom1->pos());
      Vector3d v2(*atom2->pos());
      Vector3d d = v2 - v1;
      d.normalize();
      Vector3d v3((v1 + v2 + d*(radius(atom1) - radius(atom2))) / 2);

      double shift = 0.15;
      int order = 1;
      if (m_showMulti) order = b->order();

      // The "inner" bond has to be rendered first.
      if (m_alpha < 0.999 && m_alpha > 0.001) {
        map->setFromPrimitive(atom1);
        map->setAlpha(m_alpha);
        pd->painter()->setColor( map );
        pd->painter()->drawMultiCylinder( v1, v3, m_bondRadius, order, shift );

        map->setFromPrimitive(atom2);
        map->setAlpha(m_alpha);
        pd->painter()->setColor( map );
        pd->painter()->drawMultiCylinder( v3, v2, m_bondRadius, order, shift );
      }

      // Render the selected bond.
      if (pd->isSelected(b)) {
        map->setToSelectionColor();
        pd->painter()->setColor(map);
        pd->painter()->drawMultiCylinder( v1, v2,
                           SEL_BOND_EXTRA_RADIUS + m_bondRadius, order, shift );
      }
    }
    return true;
  }

  bool BSDYEngine::renderQuick(PainterDevice *pd)
  {
    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map
    Color cSel;
    cSel.setToSelectionColor();

    // Render the bonds
    foreach(Bond *b, bonds()) {
      Atom* atom1 = pd->molecule()->atomById(b->beginAtomId());
      Atom* atom2 = pd->molecule()->atomById(b->endAtomId());
      Vector3d v1(*atom1->pos());
      Vector3d v2(*atom2->pos());
      Vector3d d = v2 - v1;
      d.normalize();
      Vector3d v3((v1 + v2 + d*(radius(atom1)-radius(atom2))) / 2);

      double shift = 0.15;
      int order = 1;
      if (m_showMulti) order = b->order();

      if (pd->isSelected(b)) {
        pd->painter()->setColor(&cSel);
        pd->painter()->drawMultiCylinder(v1, v2, SEL_BOND_EXTRA_RADIUS +
                                         m_bondRadius, order, shift);
      }
      else {
        map->setFromPrimitive(atom1);
        pd->painter()->setColor(map);
        pd->painter()->drawMultiCylinder(v1, v3, m_bondRadius, order, shift);

        map->setFromPrimitive( atom2 );
        pd->painter()->setColor(map);
        pd->painter()->drawMultiCylinder(v3, v2, m_bondRadius, order, shift);
      }
    }

    glDisable(GL_NORMALIZE);
    glEnable(GL_RESCALE_NORMAL);

    // Render the atoms
    foreach(Atom *a, atoms()) {
      if (pd->isSelected(a)) {
        pd->painter()->setColor(&cSel);
        pd->painter()->drawSphere(a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a));
      }
      else {
        map->setFromPrimitive(a);
        pd->painter()->setColor(map);
        pd->painter()->drawSphere(a->pos(), radius(a));
      }
    }

    // normalize normal vectors of bonds
    glDisable(GL_RESCALE_NORMAL);
    glEnable(GL_NORMALIZE);
    return true;
  }

  bool BSDYEngine::renderPick(PainterDevice *pd)
  {
    // Render the bonds
    foreach(Bond *b, bonds()) {
      pd->painter()->setName(b);
      // Add a slight slop factor to make it easier to pick
      // (e.g., for bond-centric tool)
      pd->painter()->drawCylinder(*b->beginPos(), *b->endPos(), m_bondRadius+0.05);
    }

    // Render the atoms
    foreach(Atom *a, atoms())  {
      pd->painter()->setName(a);
      // add a slight "slop" factor to make it easier to pick
      // (e.g., during drawing)
      // heavy atoms get a bit more, hydrogens get a bit less
      if (a->atomicNumber() > 1)
        pd->painter()->drawSphere(a->pos(), radius(a) + 0.03);
      else
        pd->painter()->drawSphere(a->pos(), radius(a) - 0.06);
    }
    return true;
  }

  // Protect globally declared functions in an anonymous namespace
  namespace
  {
    double radiusCovalent(const Atom *atom)
    {
      return OpenBabel::etab.GetCovalentRad(atom->atomicNumber());
    }

    double radiusVdW(const Atom *atom)
    {
      return OpenBabel::etab.GetVdwRad(atom->atomicNumber());
    }
  } // End of anonymous namespace

  inline double BSDYEngine::radius(const Atom *atom) const
  {
    if (atom->customRadius())
      return atom->customRadius()* m_atomRadiusPercentage;
    else {
      if (atom->atomicNumber())
        return pRadius(atom) * m_atomRadiusPercentage;
    }
    return m_atomRadiusPercentage;
  }

  void BSDYEngine::setAtomRadiusPercentage( int percent )
  {
    m_atomRadiusPercentage = 0.02 * percent;
    emit changed();
  }

  void BSDYEngine::setAtomRadiusType(int type)
  {
    m_atomRadiusType = type;
    if (type == 0)
      pRadius = radiusCovalent;
    else
      pRadius = radiusVdW;
    emit changed();
  }

  void BSDYEngine::setBondRadius( int value )
  {
    m_bondRadius = value * 0.05;
    emit changed();
  }

  void BSDYEngine::setShowMulti(int value)
  {
    m_showMulti = value;
    emit changed();
  }

  void BSDYEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    emit changed();
  }

  double BSDYEngine::radius( const PainterDevice *pd, const Primitive *p ) const
  {
    // Atom radius
    if ( p->type() == Primitive::AtomType ) {
      if ( pd ) {
        if ( pd->isSelected( p ) )
          return radius( static_cast<const Atom *>( p ) ) + SEL_ATOM_EXTRA_RADIUS;
      }
      return radius( static_cast<const Atom *>( p ) );
    }
    // Bond radius
    else if ( p->type() == Primitive::BondType ) {
      if ( pd ) {
        if ( pd->isSelected( p ) )
          return m_bondRadius + SEL_BOND_EXTRA_RADIUS;
      }
      return m_bondRadius;
    }
    // Something else
    else
      return 0.;
  }

  double BSDYEngine::transparencyDepth() const
  {
    return m_atomRadiusPercentage;
  }

  Engine::Layers BSDYEngine::layers() const
  {
    return Engine::Opaque | Engine::Transparent;
  }

  QWidget *BSDYEngine::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new BSDYSettingsWidget();
      connect(m_settingsWidget->atomRadiusSlider, SIGNAL(valueChanged(int)),
              this, SLOT(setAtomRadiusPercentage(int)));
      connect(m_settingsWidget->combo_radius, SIGNAL(currentIndexChanged(int)),
              this, SLOT(setAtomRadiusType(int)));
      connect(m_settingsWidget->bondRadiusSlider, SIGNAL(valueChanged(int)),
              this, SLOT(setBondRadius(int)));
      connect(m_settingsWidget->showMulti, SIGNAL(stateChanged(int)),
              this, SLOT(setShowMulti(int)));
      connect(m_settingsWidget->opacitySlider, SIGNAL(valueChanged(int)),
              this, SLOT(setOpacity(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
      m_settingsWidget->atomRadiusSlider->setValue(int(50*m_atomRadiusPercentage));
      m_settingsWidget->bondRadiusSlider->setValue(int(20*m_bondRadius));
      m_settingsWidget->showMulti->setCheckState((Qt::CheckState)m_showMulti);
      m_settingsWidget->opacitySlider->setValue(int(20*m_alpha));
      m_settingsWidget->combo_radius->setCurrentIndex(m_atomRadiusType);
    }
    return m_settingsWidget;
  }

  void BSDYEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  void BSDYEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("atomRadius", 50*m_atomRadiusPercentage);
    settings.setValue("radiusType", m_atomRadiusType);
    settings.setValue("bondRadius", 20*m_bondRadius);
    settings.setValue("showMulti", m_showMulti);
    settings.setValue("opacity", 20*m_alpha);
  }

  void BSDYEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setAtomRadiusPercentage(settings.value("atomRadius", 25).toInt());
    setBondRadius(settings.value("bondRadius", 2).toInt());
    setShowMulti(settings.value("showMulti", 2).toInt());
    setOpacity(settings.value("opacity", 100).toInt());
    setAtomRadiusType(settings.value("radiusType", 1).toInt());

    if (m_settingsWidget) {
      m_settingsWidget->atomRadiusSlider->setValue(int(50*m_atomRadiusPercentage));
      m_settingsWidget->combo_radius->setCurrentIndex(m_atomRadiusType);
      m_settingsWidget->bondRadiusSlider->setValue(int(20*m_bondRadius));
      m_settingsWidget->showMulti->setCheckState((Qt::CheckState)m_showMulti);
      m_settingsWidget->opacitySlider->setValue(int(20*m_alpha));
      m_settingsWidget->combo_radius->setCurrentIndex(m_atomRadiusType);
    }
  }

}

#include "bsdyengine.moc"
// This is a static engine...
// Q_EXPORT_PLUGIN2( bsdyengine, Avogadro::BSDYEngineFactory )
