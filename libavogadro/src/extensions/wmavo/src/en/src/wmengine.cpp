
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  Some portions :
  Copyright (C) 2007 Donald Ephraim Curtis

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


#include "wmengine.h"

namespace Avogadro
{

  const GLfloat WmEngine::m_mat_ambient[]={ 0.2, 0.2, 0.2, 1.0 } ;
  const GLfloat WmEngine::m_mat_diffuse[]={ 0.7, 0.7, 0.7, 1.0 } ;
  const GLfloat WmEngine::m_mat_specular[]={ 0.1, 0.1, 0.1, 1.0 } ;
  const GLfloat WmEngine::m_mat_shininess[]={ 20.0 } ;

  const GLfloat WmEngine::m_light_position[]={ 0.0, 5.0, 20.0, 1.0 } ;

  const GLfloat WmEngine::m_light_ambient0[]={ 1.0, 0.0, 0.0, 1.0 } ;
  const GLfloat WmEngine::m_light_diffuse0[]={ 1.0, 0.0, 0.0, 1.0 } ;
  const GLfloat WmEngine::m_light_specular0[]={ 1.0, 0.0, 0.0, 1.0 } ;

  const GLfloat WmEngine::m_light_ambient1[]={ 0.0, 1.0, 1.0, 1.0 } ;
  const GLfloat WmEngine::m_light_diffuse1[]={ 0.0, 1.0, 1.0, 1.0 } ;
  const GLfloat WmEngine::m_light_specular1[]={ 0.0, 1.0, 1.0, 1.0 } ;

  const double WmEngine::frustumValue::wScreenPx=1440 ;
  const double WmEngine::frustumValue::hScreenPx=900 ;
  const double WmEngine::frustumValue::wScreenCm=36.4 ;
  const double WmEngine::frustumValue::hScreenCm=22.7 ;
  const double WmEngine::frustumValue::gapEyeCamGL=0.08 ; // 1.3  ;//0.8 ; // == 2 * gapEyeCm
  const double WmEngine::frustumValue::gapEyeCm=8.0 ;
  const double WmEngine::frustumValue::xFactor=1.0 ;
  const double WmEngine::frustumValue::yFactor=1.0 ;

  const double WmEngine::radiusAtom=0.5 ;
  const int WmEngine::slicesAtom=10 ;
  const int WmEngine::stacksAtom=10 ;


  // ???
  // our sort function
  /*
  Camera *camera=0 ;

  bool sortCameraFarthest( const Primitive* lhs, const Primitive* rhs )
  {
    if ( !lhs )
    {
      if ( rhs )
        return true ;
      else
        return false ;
    }

    if( lhs->type()==Primitive::BondType
        && rhs->type()==Primitive::BondType )
    {
      if( camera )
      {
        const Bond *l = static_cast<const Bond *>( lhs );
        const Bond *r = static_cast<const Bond *>( rhs );

        const Atom* latom1=static_cast<const Atom *>( l->GetBeginAtom() );
        const Atom* latom2=static_cast<const Atom *>( l->GetEndAtom() );
        Vector3d lv1( latom1->pos() );
        Vector3d lv2( latom2->pos() );
        Vector3d ld1 = lv2 - lv1;
        ld1.normalize();

        const Atom* ratom1=static_cast<const Atom *>( r->GetBeginAtom() );
        const Atom* ratom2=static_cast<const Atom *>( r->GetEndAtom() );
        Vector3d rv1( ratom1->pos() );
        Vector3d rv2( ratom2->pos() );
        Vector3d rd1 = rv2 - rv1;
        return camera->distance( ld1 ) >= camera->distance( rd1 );
      }

    } else if ( lhs->type()==Primitive::AtomType
                && rhs->type()==Primitive::AtomType )
    {
      if( camera )
      {
        const Atom *l = static_cast<const Atom*>( lhs );
        const Atom *r = static_cast<const Atom*>( rhs );
        return camera->distance(l->pos()) >= camera->distance(r->pos()) ;
      }
    }

    return false;
  }
  */


  /**
    * Constructor.
    * @param parent Instanciate and initiate by Avogadro
    */
  WmEngine::WmEngine(QObject *parent) : Engine(parent), m_widget(NULL),
      /*m_settingsWidget(0),*/ m_atomRadiusPercentage(0.3), m_bondRadius(0.1),
      m_showMulti(2)/*, m_alpha(1.)*/
  {

    quadricAtom = gluNewQuadric() ;
    gluQuadricDrawStyle( quadricAtom, GLU_FILL ) ;

    // -> 3d-stereo
    m_frust.wWidgetPx = 0.0 ;
    m_frust.hWidgetPx = 0.0 ;
    m_frust.wWidgetCm = 0.0 ;
    m_frust.hWidgetCm = 0.0 ;

    m_frust.aspectRatioWidget = 0.0 ;

    m_frust.zScreen = 0.0 ;
    m_frust.zNear = 0.0 ; // 7.0, 0.01
    m_frust.zFar = 0.0 ; // 13.0, 50
    m_frust.fov = 0.0 ;
    // <-
  }


  /**
    * Destructor.
    */
  WmEngine::~WmEngine()
  {
    gluDeleteQuadric( quadricAtom ) ;

    /*
    if ( m_settingsWidget ) {
      m_settingsWidget->deleteLater();
    }
    */
  }


  /**
    * Clone the current engine.
    */
  Engine *WmEngine::clone() const
  {
    /*
    WmEngine *engine = new WmEngine(parent());
    engine->setAlias(alias());
    engine->m_atomRadiusPercentage = m_atomRadiusPercentage;
    engine->m_bondRadius = m_bondRadius;
    engine->m_showMulti = m_showMulti;
    //engine->m_alpha = m_alpha;
    engine->setEnabled(isEnabled());

    return engine;
    */
    return NULL ;
  }

  /**
    * Render the scene in the "opaque mode".
    * @return TRUE if the rendering was completed successfully ; else FALSE.
    * @param pd Instantiate and initiate by Avogadro. It is the GLPainterDevice objet of the GLWiget class.
    */
  bool WmEngine::renderOpaque( PainterDevice *pd )
  {
    if( m_widget == NULL )
    {
      m_widget = pd->camera()->parent() ;
      m_cam = pd->camera() ;
    }

    displayStereoAnag() ;

    return true;
  }


  /**
    * Render the scene in the "transparence mode".
    * @return TRUE if the rendering was completed successfully ; else FALSE.
    * @param pd Instantiate and initiate by Avogadro. It is the GLPainterDevice objet of the GLWiget class.
    */
  bool WmEngine::renderTransparent(PainterDevice *pd)
  {

    if( m_widget == NULL )
    {
      m_widget = pd->camera()->parent() ;
      m_cam = pd->camera() ;
    }

    displayStereoAnag() ;

    /*
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
    */

    return true;
  }



  /**
    * Render the scene in the "quick mode".
    * @return TRUE if the rendering was completed successfully ; else FALSE.
    * @param pd Instantiate and initiate by Avogadro. It is the GLPainterDevice objet of the GLWiget class.
    */
  bool WmEngine::renderQuick(PainterDevice *pd)
  {

    if( m_widget == NULL )
    {
      m_widget = pd->camera()->parent() ;
      m_cam = pd->camera() ;
    }

    //displayStereoAnag() ;

    /*
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
    */


    return true;
  }


  /**
    * Render the scene in the "pick mode".
    * @return TRUE if the rendering was completed successfully ; else FALSE.
    * @param pd Instantiate and initiate by Avogadro. It is the GLPainterDevice objet of the GLWiget class.
    */
  bool WmEngine::renderPick(PainterDevice *pd)
  {

    if( m_widget == NULL )
    {
      m_widget = pd->camera()->parent() ;
      m_cam = pd->camera() ;
    }

    displayStereoAnag() ;

    /*
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
    */

    return true;
  }


  /**
    * Get the size of an atom.
    * @return Size of an atom.
    * @see radius(const PainterDevice *pd, const Primitive *p = 0)
    */
  inline double WmEngine::radius(const Atom *atom) const
  {
    if (atom->atomicNumber())
      return OpenBabel::etab.GetVdwRad(atom->atomicNumber()) * m_atomRadiusPercentage;
    return m_atomRadiusPercentage;
  }

  /*
  void WmEngine::setAtomRadiusPercentage( int percent )
  {
    m_atomRadiusPercentage = 0.02 * percent;
    emit changed();
  }

  void WmEngine::setBondRadius( int value )
  {
    m_bondRadius = value * 0.05;
    emit changed();
  }

  void WmEngine::setShowMulti(int value)
  {
    m_showMulti = value;
    emit changed();
  }

  void WmEngine::setOpacity(int value)
  {
    m_alpha = 0.05 * value;
    emit changed();
  }
  */


  /**
    * Get the radius of the primitive in parameter.
    * @return The radius of the primitive.
    * @param pd Instantiate and initiate by Avogadro. It is the GLPainterDevice objet of the GLWiget class.
    * @param p The primitive ...
    */
  double WmEngine::radius( const PainterDevice *pd, const Primitive *p ) const
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

  /*
  double WmEngine::transparencyDepth() const
  {
    return m_atomRadiusPercentage;
  }
  */

  /**
    * The layers ... A mystic method in fact.
    * @return The layers ... But why ?
    */
  Engine::Layers WmEngine::layers() const
  {
    return Engine::Opaque | Engine::Transparent;
  }

  /*
  QWidget *WmEngine::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new WMSettingsWidget();
      connect(m_settingsWidget->atomRadiusSlider, SIGNAL(valueChanged(int)),
              this, SLOT(setAtomRadiusPercentage(int)));
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
    }
    return m_settingsWidget;
  }
  */

  /*
  void WmEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }
  */

  /*
  void WmEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("atomRadius", 50*m_atomRadiusPercentage);
    settings.setValue("bondRadius", 20*m_bondRadius);
    settings.setValue("showMulti", m_showMulti);
    settings.setValue("opacity", 20*m_alpha);
  }
  */

  /*
  void WmEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setAtomRadiusPercentage(settings.value("atomRadius", 3).toInt());
    setBondRadius(settings.value("bondRadius", 2).toInt());
    setShowMulti(settings.value("showMulti", 2).toInt());
    setOpacity(settings.value("opacity", 100).toInt());

    if (m_settingsWidget) {
      m_settingsWidget->atomRadiusSlider->setValue(int(50*m_atomRadiusPercentage));
      m_settingsWidget->bondRadiusSlider->setValue(int(20*m_bondRadius));
      m_settingsWidget->showMulti->setCheckState((Qt::CheckState)m_showMulti);
      m_settingsWidget->opacitySlider->setValue(int(20*m_alpha));
    }
  }
  */


  /**
    * Initialization before realize the render.
    */
  void WmEngine::begin()
  {

    // paintGL()

    glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
    glMatrixMode( GL_PROJECTION ) ;
    glPushMatrix() ;
    glMatrixMode( GL_MODELVIEW ) ;
    glPushMatrix() ;


    // initGL()
    glDisable( GL_DITHER ) ;
    //glDisable( GL_ALPHA ) ; // test sans.
    glDisable( GL_BLEND ) ; // inutilisé lors de la stéréo.

    //glClearColor( 0, 0, 0, 1 ) ;
    QColor backColor=m_widget->background() ;
    glClearColor( backColor.redF(), backColor.greenF(), backColor.blueF(), 1 ) ;

    glShadeModel( GL_SMOOTH ) ;

    //glEnable( GL_STENCIL_TEST ) ; // NE PAS ACTIVER !! Sinon problème d'affichage : les sphères ne s'affiche pas complètment, il manque des morceaux.
    glEnable( GL_DEPTH_TEST ) ;
    glEnable( GL_CULL_FACE ) ;
    glEnable( GL_NORMALIZE ) ;
    glEnable( GL_LIGHTING ) ;

    glMaterialfv( GL_FRONT, GL_AMBIENT, m_mat_ambient ) ;
    glMaterialfv( GL_FRONT, GL_DIFFUSE, m_mat_diffuse ) ;
    glMaterialfv( GL_FRONT, GL_SPECULAR, m_mat_specular ) ;
    glMaterialfv( GL_FRONT, GL_SHININESS, m_mat_shininess ) ;

    glDisable( GL_LIGHT0 ) ;
    glLightfv( GL_LIGHT0, GL_POSITION, m_light_position ) ;
    glLightfv( GL_LIGHT0, GL_AMBIENT, m_light_ambient0 ) ;
    glLightfv( GL_LIGHT0, GL_DIFFUSE, m_light_diffuse0 ) ;
    glLightfv( GL_LIGHT0, GL_SPECULAR, m_light_specular0 ) ;

    glDisable( GL_LIGHT1 ) ;
    glLightfv( GL_LIGHT1, GL_POSITION, m_light_position ) ;
    glLightfv( GL_LIGHT1, GL_AMBIENT, m_light_ambient1 ) ;
    glLightfv( GL_LIGHT1, GL_DIFFUSE, m_light_diffuse1 ) ;
    glLightfv( GL_LIGHT1, GL_SPECULAR, m_light_specular1 ) ;
  }



  /**
    * Re-Initialization after realize the render to come back in the "old values".
    */
  void WmEngine::end()
  {

    // voir initGL()

    /* on verra
    if( m_widget )
    {
      //qglClearColor( m_widget->background() ) ;
      glClearColor( 0, 0, 0, 1 ) ;
    }
    else
      glClearColor( 0, 0, 0, 1 ) ;
      */

    //glDisable( GL_DITHER ) ; // Reste comme c
    //glShadeModel( GL_SMOOTH ) ; // Reste comme c

    //glEnable( GL_DEPTH_TEST ) ; // Reste comme c
    //glEnable( GL_CULL_FACE ) ; // Reste comme c
    //glEnable( GL_NORMALIZE ) ; // Reste comme c

    //glDisable( GL_BLEND ) ; // Activé pour la méthode renderTransparancy()
    //glEnable( GL_LIGHTING ) ;  // Reste comme c

    //glMaterialfv( GL_FRONT, GL_AMBIENT, m_mat_ambient ) ; // Reste comme c
    //glMaterialfv( GL_FRONT, GL_DIFFUSE, m_mat_diffuse ) ; // Reste comme c
    //glMaterialfv( GL_FRONT, GL_SPECULAR, m_mat_specular ) ; // Reste comme c
    //glMaterialfv( GL_FRONT, GL_SHININESS, m_mat_shininess ) ; // Reste comme c

    glDisable( GL_LIGHT0 ) ;
    glLightfv( GL_LIGHT0, GL_POSITION, LIGHT0_POSITION ) ;
    glLightfv( GL_LIGHT0, GL_AMBIENT, LIGHT_AMBIENT ) ;
    glLightfv( GL_LIGHT0, GL_DIFFUSE, LIGHT0_DIFFUSE ) ;
    glLightfv( GL_LIGHT0, GL_SPECULAR, LIGHT0_SPECULAR ) ;
    glEnable( GL_LIGHT0 ) ;

    glDisable( GL_LIGHT1 ) ;
    glLightfv( GL_LIGHT1, GL_POSITION, LIGHT1_POSITION ) ;
    glLightfv( GL_LIGHT1, GL_AMBIENT, LIGHT_AMBIENT ) ;
    glLightfv( GL_LIGHT1, GL_DIFFUSE, LIGHT1_DIFFUSE ) ;
    glLightfv( GL_LIGHT1, GL_SPECULAR, LIGHT1_SPECULAR ) ;
    glEnable( GL_LIGHT1 ) ;


    // voir paintGL() :

    // Surtout pas !!
    //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

    //glMatrixMode( GL_MODELVIEW ) ;
    glPopMatrix() ;
    glMatrixMode( GL_PROJECTION ) ;
    glPopMatrix() ;
    glPopAttrib() ;
    glMatrixMode( GL_MODELVIEW ) ;
  }


  /**
    * The render of anaglyph.
    */
  void WmEngine::displayStereoAnag()
  {


    // Ajouté la possibilité de ne plus visualiser les atomes d'Hydrogène
    // Voir pour ajouter la fonctionnalité aussi dans le "moteur de rendu de base"

    GLfloat projectionMatrix[16] ;

    /*
    glGetFloatv( GL_PROJECTION_MATRIX, projectionMatrix ) ;

    cout << "1 Projection Matrix:" << endl ;
    cout<<" "<<projectionMatrix[0]<<" "<<projectionMatrix[4]<<" "<<projectionMatrix[8]<<" "<<projectionMatrix[12]<<endl;
    cout<<" "<<projectionMatrix[1]<<" "<<projectionMatrix[5]<<" "<<projectionMatrix[9]<<" "<<projectionMatrix[13]<<endl;
    cout<<" "<<projectionMatrix[2]<<" "<<projectionMatrix[6]<<" "<<projectionMatrix[10]<<" "<<projectionMatrix[14]<<endl;
    cout<<" "<<projectionMatrix[3]<<" "<<projectionMatrix[7]<<" "<<projectionMatrix[11]<<" "<<projectionMatrix[15]<<endl;
*/



    begin() ;

    int i=0 ;
    double posEye=0.0 ;

    glClear(GL_COLOR_BUFFER_BIT); // we must clear each buffer now.

    for( i=0 ; i<2 ; i++ )
    {
      glEnable( GL_LIGHT0 + i) ;
      glClear( GL_DEPTH_BUFFER_BIT ) ;

      if( i == 0 ) // Left eye - RED
      {
        posEye = m_frust.gapEyeCamGL ;
        glColorMask( GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE ) ;
        glStencilFunc( GL_NOTEQUAL, 1, 1 ) ; // Draws if stencil <> 1
      }
      else // Right eye - CYAN
      {
        posEye = (-1*m_frust.gapEyeCamGL) ;
        glColorMask( GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE ) ;
        glStencilFunc( GL_EQUAL, 1, 1 ) ; // Draws if stencil <> 0
      }

      /// Frustum.
/*->*/
      // Récupérer les valeurs.

      // Taille de la scène, ratio de celle-ci.
      GLint viewport[4] ;
      glGetIntegerv( GL_VIEWPORT, viewport ) ;
      m_frust.convertWWidgetPxToCm( viewport[2] ) ;
      m_frust.convertHWidgetPxToCm( viewport[3] ) ;
      m_frust.aspectRatioWidget = m_frust.wWidgetPx / m_frust.hWidgetPx ;

      // zNear, zFar, fov.
      //double molRadius=m_widget->radius() + CAMERA_MOL_RADIUS_MARGIN ; // radius() + 10 ....
      //double distToMolCenter=m_cam->distance( m_widget->center() ) ;
      //m_frust.zNear = std::max( CAMERA_NEAR_DISTANCE, distToMolCenter-molRadius ) ; // CAM_NEAR=2.0
      m_frust.zNear = 1 ; // >1 : not good, because re-define the near and it appears a avogadro bug.
                        //
      //m_frust.zFar = distToMolCenter + molRadius ;
      m_frust.zFar = 100 ;
      //m_frust.zScreen = (m_frust.zNear+m_frust.zFar) / 2 ;
      //m_frust.zScreen = (m_frust.zNear+10) ;
      m_frust.zScreen = 45 ;
      m_frust.fov = m_cam->angleOfViewY() ;
      double ratioZNearZScreen=m_frust.zNear/m_frust.zScreen ; //2.0/3.0 ;

      //cout << "ratioZNearZScreen:" << ratioZNearZScreen << endl ;


      // Left, right, bottom, top of Avogadro.


      // Méthode0 : Marche pas du tout ... Non respect des ratios à l'affichage.
      // Utilisation originale (du fichier stereo).
      //double wGL=m_frust.convertPxToGL( m_frust.wWidgetPx ) ;
      //double hGL=m_frust.convertPxToGL( m_frust.hWidgetPx ) ;
      //GLdouble leftAvo0=-((wGL/2.0)+posEye) * ratioZNearZScreen * m_frust.xFactor ;
      //GLdouble rightAvo0=((wGL/2.0)+posEye) * ratioZNearZScreen * m_frust.xFactor ;
      //GLdouble bottom0=-(hGL/2.0)* ratioZNearZScreen * m_frust.yFactor ;
      //GLdouble top0=(hGL/2.0)* ratioZNearZScreen * m_frust.yFactor ;
      //cout << "leftAvo0:" << leftAvo0 << " rightAvo0:" << rightAvo0 ;
      //cout << " bottom0:" << bottom0 << " top0:" << top0 << endl ;


      // Méthode1 : glFrustum => GL_PROJ NON identique après utilisation du glFrustum !
      // doc d'openGL 9.085
      //GLdouble top1 = tan(m_frust.fov*0.5) * m_frust.zNear ;
      //GLdouble bottom1 = -top1 ;
      //GLdouble leftAvo1 = m_frust.aspectRatioWidget * bottom1 ;
      //GLdouble rightAvo1 = m_frust.aspectRatioWidget * top1 ;
      //cout << "leftAvo1:" << leftAvo1 << " rightAvo1:" << rightAvo1 ;
      //cout << " bottom1:" << bottom1 << " top1:" << top1 << endl ;


      // Méthode 2 : glFrustum => GL_PROJ IDENTIQUE après utilisation du glFrustum !
      // www.songho.ca/opengl/gl_projectionmatrix.html
      // Dans le cas particulier où le volume de vue est symétrique.
      glGetFloatv( GL_PROJECTION_MATRIX, projectionMatrix ) ;
      // 0  4  8  12        n/r   0     0             0
      // 1  5  9  13   <=>  0     n/t   0             0
      // 2  6  10 14        0     0     -(f+t)/(f-n)  (-2.f.n)/(f-n)
      // 3  7  11 15        0     0     -1            0
      GLdouble top2 = m_frust.zNear / (double)projectionMatrix[5] ;
      GLdouble bottom2 = -top2 ;
      GLdouble leftAvo2 = (-1) * m_frust.zNear / (double)projectionMatrix[0] ;
      GLdouble rightAvo2 = -leftAvo2 ;

      //cout << "leftAvo2:" << leftAvo2 << " rightAvo2:" << rightAvo2 ;
      //cout << " bottom2:" << bottom2 << " top2:" << top2 << endl ;


      // Méthode 3 : glFrustum => GL_PROJ IDENTIQUE après utilisation du glFrustum !
      // www.songho.ca/opengl/gl_transform.html => makeFrustum()
      //const double DEG2RAD=3.14159265/180 ;
      //double tangent=tan(m_frust.fov/2 * DEG2RAD ) ;
      //GLdouble top3=m_frust.zNear * tangent ;
      //GLdouble bottom3=-top3 ;
      //GLdouble rightAvo3=top3 * m_frust.aspectRatioWidget ;
      //GLdouble leftAvo3=-rightAvo3 ;
      //cout << "leftAvo3:" << leftAvo3 << " rightAvo3:" << rightAvo3 ;
      //cout << " bottom3:" << bottom3 << " top3:" << top3 << endl ;


      // Left & right for the stereo.

      // Semble correct dans la visu.
      // Erreur dans la philosophie du papier car,
      // selon le code stereo original, l'écart entre les yeux est fixé sur le
      // plan zSreen, puis un ratio est appliqué pour obtenir les valeurs sur le
      // plan zNear.
      // Réglage 1
      //GLdouble leftStereo = (leftAvo2 + posEye) * ratioZNearZScreen ;
      //GLdouble rightStereo = (rightAvo2 + posEye) * ratioZNearZScreen ;
      //GLdouble topStereo = top2 * ratioZNearZScreen ;
      //GLdouble bottomStereo = bottom2 * ratioZNearZScreen ;


      // Pour mettre en place l'impression d'une image qui "sort" (inversion des couleurs
      // rouge/cyan), un plan zScreen (plan sur lequel l'inversion s'effectuera) doit être
      // mis en place. Pour cela, il est nécessaire de respecter l'idée que l'écart des
      // yeux (6 à 8 cm) se trouve au niveau du plan zScreen. Donc, l'écart des yeux au niveau
      // du plan zNear est (la distance de l'écart des yeux) * (zNear/zScreen).
      // Réglage 2
      GLdouble leftStereo = leftAvo2 + (posEye*ratioZNearZScreen) ;
      GLdouble rightStereo = rightAvo2 + (posEye*ratioZNearZScreen) ;
      GLdouble topStereo = top2 ;
      GLdouble bottomStereo = bottom2 ;


      //cout << "leftStereo:" << leftStereo << " rightStereo:" << rightStereo << endl ;



      //cout << "zNear:" << m_frust.zNear << " zFar:" << m_frust.zFar << " far-near:" << (m_frust.zFar-m_frust.zNear) << endl;
      //cout << " zScreen" << m_frust.zScreen << endl ;
      //cout << endl ;




      glMatrixMode( GL_PROJECTION ) ;
      glPushMatrix() ;
      glLoadIdentity() ;


      // Valeurs dans GL_PROJ identique avant et après !
      // Bonnes valeurs trouvées.
      //gluPerspective( m_frust.fov, m_frust.aspectRatioWidget, m_frust.zNear, m_frust.zFar );

      // Valeurs dans GL_PROJ identique avant et après !
      // Bonnes valeurs trouvées.
      //glFrustum( leftAvo2, rightAvo2, bottom2, top2, m_frust.zNear, m_frust.zFar ) ;

      glFrustum( leftStereo, rightStereo, bottomStereo, topStereo, m_frust.zNear, m_frust.zFar ) ;


      //glGetFloatv( GL_PROJECTION_MATRIX, projectionMatrix ) ;

      //cout << "2 Projection Matrix:" << endl ;
      //cout<<" "<<projectionMatrix[0]<<" "<<projectionMatrix[4]<<" "<<projectionMatrix[8]<<" "<<projectionMatrix[12]<<endl;
      //cout<<" "<<projectionMatrix[1]<<" "<<projectionMatrix[5]<<" "<<projectionMatrix[9]<<" "<<projectionMatrix[13]<<endl;
      //cout<<" "<<projectionMatrix[2]<<" "<<projectionMatrix[6]<<" "<<projectionMatrix[10]<<" "<<projectionMatrix[14]<<endl;
      //cout<<" "<<projectionMatrix[3]<<" "<<projectionMatrix[7]<<" "<<projectionMatrix[11]<<" "<<projectionMatrix[15]<<endl;

      //GLenum errCode ;
      //const GLubyte *errString ;
      //if( (errCode=glGetError()) != GL_NO_ERROR )
      //{
        //errString = gluErrorString( errCode ) ;
        //fprintf (stderr, "OpenGL Error: %s\n", errString);
      //}

/*<-*/



      /// Translate for eyes.

      GLfloat modelviewMatrix[16] ;
      glGetFloatv( GL_MODELVIEW_MATRIX, modelviewMatrix ) ;


      //cout << "1 Projection Matrix:" << endl ;
      //cout<<" "<<modelviewMatrix[0]<<" "<<modelviewMatrix[4]<<" "<<modelviewMatrix[8]<<" "<<modelviewMatrix[12]<<endl;
      //cout<<" "<<modelviewMatrix[1]<<" "<<modelviewMatrix[5]<<" "<<modelviewMatrix[9]<<" "<<modelviewMatrix[13]<<endl;
      //cout<<" "<<modelviewMatrix[2]<<" "<<modelviewMatrix[6]<<" "<<modelviewMatrix[10]<<" "<<modelviewMatrix[14]<<endl;
      //cout<<" "<<modelviewMatrix[3]<<" "<<modelviewMatrix[7]<<" "<<modelviewMatrix[11]<<" "<<modelviewMatrix[15]<<endl;

      // Lecture de la matrice en colonne !! ou je ne sais tjs pas ...
      //float rightScreen[3]={modelviewMatrix[0], modelviewMatrix[1], modelviewMatrix[2]} ;
      float rightScreen[3]={modelviewMatrix[0], modelviewMatrix[4], modelviewMatrix[8]} ;
      Vector3d xAxis=m_cam->backTransformedXAxis() ;
      //cout << xAxis[0] << " " << xAxis[1] << " " << xAxis[2] << endl ;

      glMatrixMode( GL_MODELVIEW ) ;
      glPushMatrix() ;
      glTranslatef( rightScreen[0]*posEye, rightScreen[1]*posEye, rightScreen[2]*posEye ) ;

      //-> My object.
      drawAtomBond() ;
      //<- My object.

      // MODELVIEW.
      glPopMatrix() ;

/*->*/
      glMatrixMode( GL_PROJECTION ) ;
      glPopMatrix() ;
      glMatrixMode( GL_MODELVIEW ) ;
/*<-*/


      glDisable(GL_LIGHT0 + i);
    }

    //cout << "END END !!" << endl ;

    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    end() ;

    //const_cast<GLWidget*>(m_widget)->update();
  }


  /**
    * Render the atoms and the bonds of the scene.
    */
  void WmEngine::drawAtomBond()
  {
    glPushAttrib(GL_ALL_ATTRIB_BITS);


    //glDisable( GL_NORMALIZE ) ;
    //glEnable( GL_RESCALE_NORMAL ) ;

    foreach( const Atom *a, atoms() )
    {
      glPushMatrix() ;

      Vector3d pos=*(a->pos()) ;
      glTranslatef( pos[0], pos[1], pos[2] ) ;
      gluSphere( quadricAtom, radiusAtom, slicesAtom, stacksAtom ) ;

      glPopMatrix() ;
    }

    //glDisable( GL_RESCALE_NORMAL ) ;
    //glEnable( GL_NORMALIZE ) ;

    glLineWidth( 10.0F ) ;

    foreach( const Bond *b, bonds() )
    {
      Atom* atom1 = m_widget->molecule()->atomById(b->beginAtomId());
      Atom* atom2 = m_widget->molecule()->atomById(b->endAtomId());

      if (!atom1 || !atom2)
      {
        qDebug() << "Invalid bond atom IDs" << b->beginAtomId() << atom1
                 << b->endAtomId() << atom2 << "Bond" << b->id();
        continue;
      }
      else
      {
        glPushMatrix() ;

        Vector3d pos1=*(atom1->pos()) ;
        Vector3d pos2=*(atom2->pos()) ;

        glBegin( GL_LINES ) ;
          glColor3f( 1.0, 0.0, 0.0 ) ;
          glVertex3f( pos1[0], pos1[1], pos1[2] ) ;
          glColor3f( 0.0, 0.5, 0.5 ) ;
          glVertex3f( pos2[0], pos2[1], pos2[2] ) ;
        glEnd() ;

        glPopMatrix() ;
      }
    }


    glPopAttrib() ;
  }
}

//#include "WMengine.moc"
// This is a static engine...
Q_EXPORT_PLUGIN2( WmEngine, Avogadro::WmEngineFactory )
