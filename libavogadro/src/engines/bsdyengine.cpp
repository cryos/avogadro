/**********************************************************************
  BSDYEngine - Dynamic detail engine for "balls and sticks" display

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include <config.h>
#include "bsdyengine.h"
#include <avogadro/camera.h>
#include <avogadro/painter.h>
#include <avogadro/color.h>

#include <QGLWidget> // for OpenGL bits
#include <QDebug>

using namespace std;
using namespace OpenBabel;
using namespace Eigen;

namespace Avogadro
{

// our sort function
  Camera *camera = 0;
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
  }

  BSDYEngine::BSDYEngine( QObject *parent ) : Engine( parent ),
      m_settingsWidget( 0 ), m_atomRadiusPercentage( 0.3 ), m_bondRadius( 0.1 ),
      m_showMulti(true)
  {
    setDescription( tr( "Renders primitives using Balls (atoms) and Sticks (bonds)." ) );

  }

  Engine *BSDYEngine::clone() const
  {
    BSDYEngine *engine = new BSDYEngine(parent());
    engine->setName(name());
    engine->m_atomRadiusPercentage = m_atomRadiusPercentage;
    engine->m_bondRadius = m_bondRadius;
    engine->m_showMulti = m_showMulti;
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
    QList<Primitive *> list;

    glPushAttrib( GL_TRANSFORM_BIT );

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    // Get a list of bonds and render them
    list = primitives().subList( Primitive::BondType );

    foreach( Primitive *p, list ) {
      const Bond *b = static_cast<const Bond *>( p );

      const Atom* atom1 = static_cast<const Atom *>( b->GetBeginAtom() );
      const Atom* atom2 = static_cast<const Atom *>( b->GetEndAtom() );
      Vector3d v1( atom1->pos() );
      Vector3d v2( atom2->pos() );
      Vector3d d = v2 - v1;
      d.normalize();
      Vector3d v3(( v1 + v2 + d*( radius( atom1 )-radius( atom2 ) ) ) / 2 );

      double shift = 0.15;
      int order = 1;
      if (m_showMulti) order = b->GetBO();

      map->set( atom1 );
      pd->painter()->setColor( map );
      pd->painter()->setName( b );
      pd->painter()->drawMultiCylinder( v1, v3, m_bondRadius, order, shift );

      map->set( atom2 );
      pd->painter()->setColor( map );
      pd->painter()->setName( b );
      pd->painter()->drawMultiCylinder( v3, v2, m_bondRadius, order, shift );
    }

    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );

    // Build up a list of the atoms and render them
    list = primitives().subList( Primitive::AtomType );
    foreach( Primitive *p, list ) {
      const Atom *a = static_cast<const Atom *>( p );

      map->set( a );
      pd->painter()->setColor( map );
      pd->painter()->setName( a );
      pd->painter()->drawSphere( a->pos(), radius( a ) );
    }

    // normalize normal vectors of bonds
    glDisable( GL_RESCALE_NORMAL );
    glEnable( GL_NORMALIZE );

    glPopAttrib();

    return true;
  }

  bool BSDYEngine::renderTransparent( PainterDevice *pd )
  {
    QList<Primitive *> list;

    camera = pd->camera();

    glPushAttrib( GL_TRANSFORM_BIT );

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map

    // Get a list of bonds and render them
    list = primitives().subList( Primitive::BondType );

    // sort our atom list
    // qSort(list.begin(), list.end(), sortCameraFarthest);

    // enable depth mast for bonds
    glDepthMask( GL_TRUE );

    // push bond type
    foreach( Primitive *p, list ) {
      const Bond *b = static_cast<const Bond *>( p );

      // Render the selection highlight
      if ( pd->isSelected( b ) ) {
        const Atom* atom1 = static_cast<const Atom *>( b->GetBeginAtom() );
        const Atom* atom2 = static_cast<const Atom *>( b->GetEndAtom() );
        Vector3d v1( atom1->pos() );
        Vector3d v2( atom2->pos() );

        double shift = 0.15;
        int order = b->GetBO();

        map->setToSelectionColor();
        glEnable( GL_BLEND );
        pd->painter()->setColor( map );
        pd->painter()->setName( b );
        if (order == 1)
          pd->painter()->drawCylinder(v1, v2, SEL_BOND_EXTRA_RADIUS + m_bondRadius);
        else
          pd->painter()->drawMultiCylinder( v1, v2, SEL_BOND_EXTRA_RADIUS + m_bondRadius, order, shift );
        glDisable( GL_BLEND );
      }
    }

    glDepthMask( GL_FALSE );
    glDisable( GL_NORMALIZE );
    glEnable( GL_RESCALE_NORMAL );

    // Build up a list of the atoms and render them
    list = primitives().subList( Primitive::AtomType );

    // sort our atom list
    // qSort(list.begin(), list.end(), sortCameraFarthest);

    foreach( Primitive *p, list ) {
      const Atom *a = static_cast<const Atom *>( p );

      // Render the selection highlight
      if ( pd->isSelected( a ) ) {
        map->setToSelectionColor();
        glEnable( GL_BLEND );
        pd->painter()->setColor( map );
        pd->painter()->setName( a );
        pd->painter()->drawSphere( a->pos(), SEL_ATOM_EXTRA_RADIUS + radius( a ) );
        glDisable( GL_BLEND );
      }
    }

    // normalize normal vectors of bonds
    glDisable( GL_RESCALE_NORMAL );
    glEnable( GL_NORMALIZE );

    glPopAttrib();

    return true;
  }

  bool BSDYEngine::renderQuick(PainterDevice *pd)
  {
    // Render atoms and bond with no transparency...
    QList<Primitive *> list;

    Color *map = colorMap(); // possible custom color map
    if (!map) map = pd->colorMap(); // fall back to global color map
    Color cSel;
    cSel.setToSelectionColor();

    // Get a list of bonds and render them
    list = primitives().subList(Primitive::BondType);

    foreach(Primitive *p, list)
    {
      const Bond *b = static_cast<const Bond *>(p);

      const Atom* atom1 = static_cast<const Atom *>(b->GetBeginAtom());
      const Atom* atom2 = static_cast<const Atom *>(b->GetEndAtom());
      Vector3d v1(atom1->pos());
      Vector3d v2(atom2->pos());
      Vector3d d = v2 - v1;
      d.normalize();
      Vector3d v3((v1 + v2 + d*(radius(atom1)-radius(atom2))) / 2);

      if (pd->isSelected(b))
      {
        pd->painter()->setColor(&cSel);
        pd->painter()->setName(b);
        pd->painter()->drawCylinder(v1, v2, SEL_BOND_EXTRA_RADIUS +
                                    m_bondRadius);
      }
      else
      {
        map->set(atom1);
        pd->painter()->setColor(map);
        pd->painter()->setName(b);
        pd->painter()->drawCylinder(v1, v3, m_bondRadius);

        map->set( atom2 );
        pd->painter()->setColor(map);
        pd->painter()->setName(b);
        pd->painter()->drawCylinder(v3, v2, m_bondRadius);
      }
    }

    glDisable(GL_NORMALIZE);
    glEnable(GL_RESCALE_NORMAL);

    // Build up a list of the atoms and render them
    list = primitives().subList(Primitive::AtomType);
    foreach(Primitive *p, list)
    {
      const Atom *a = static_cast<const Atom *>(p);

      if (pd->isSelected(a))
      {
        pd->painter()->setColor(&cSel);
        pd->painter()->setName(a);
        pd->painter()->drawSphere(a->pos(), SEL_ATOM_EXTRA_RADIUS + radius(a));
      }
      else
      {
        map->set(a);
        pd->painter()->setColor(map);
        pd->painter()->setName(a);
        pd->painter()->drawSphere(a->pos(), radius(a));
      }
    }

    // normalize normal vectors of bonds
    glDisable(GL_RESCALE_NORMAL);
    glEnable(GL_NORMALIZE);
    return true;
  }

  inline double BSDYEngine::radius( const Atom *atom ) const
  {
    return etab.GetVdwRad( atom->GetAtomicNum() ) * m_atomRadiusPercentage;
  }

  void BSDYEngine::setAtomRadiusPercentage( int percent )
  {
    m_atomRadiusPercentage = 0.1 * percent;
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

  QWidget *BSDYEngine::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new BSDYSettingsWidget();
      connect(m_settingsWidget->atomRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(setAtomRadiusPercentage(int)));
      connect(m_settingsWidget->bondRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(setBondRadius(int)));
      connect(m_settingsWidget->showMulti, SIGNAL(stateChanged(int)), this, SLOT(setShowMulti(int)));
      connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
      m_settingsWidget->atomRadiusSlider->setValue(10*m_atomRadiusPercentage);
      m_settingsWidget->bondRadiusSlider->setValue(20*m_bondRadius);
      m_settingsWidget->showMulti->setCheckState((Qt::CheckState)m_showMulti);
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
    settings.setValue("atomRadius", 10*m_atomRadiusPercentage);
    settings.setValue("bondRadius", 20*m_bondRadius);
    settings.setValue("showMulti", m_showMulti);
  }

  void BSDYEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setAtomRadiusPercentage(settings.value("atomRadius", 3).toInt());
    setBondRadius(settings.value("bondRadius", 2).toInt());
    setShowMulti(settings.value("showMulti", 2).toInt());

    if (m_settingsWidget) {
      m_settingsWidget->atomRadiusSlider->setValue(10*m_atomRadiusPercentage);
      m_settingsWidget->bondRadiusSlider->setValue(20*m_bondRadius);
      m_settingsWidget->showMulti->setCheckState((Qt::CheckState)m_showMulti);
    }
  }


  Engine::EngineFlags BSDYEngine::flags() const
  {
    return Engine::Transparent | Engine::Atoms | Engine::Bonds;
  }
//   AVOGADRO_ENGINE_FACTORY(BSDYEngine)

}

#include "bsdyengine.moc"

Q_EXPORT_PLUGIN2( bsdyengine, Avogadro::BSDYEngineFactory )
