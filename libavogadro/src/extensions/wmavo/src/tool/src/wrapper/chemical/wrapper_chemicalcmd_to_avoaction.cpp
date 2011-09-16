  
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  Some portions :
  Copyright (C) 2007-2009 Marcus D. Hanwell
  Copyright (C) 2006,2008,2009 Geoffrey R. Hutchison
  Copyright (C) 2006,2007 Donald Ephraim Curtis
  Copyright (C) 2008,2009 Tim Vandermeersch

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

#include "wrapper_chemicalcmd_to_avoaction.h"

#include "wmtool.h"
#include "moleculemanipulation.h"
#include "contextmenu_to_avoaction.h"

namespace Avogadro
{

  const Eigen::Vector3d WrapperChemicalCmdToAvoAction::m_vect3d0(Eigen::Vector3d(0., 0., 0.)) ;
  Eigen::Transform3d WrapperChemicalCmdToAvoAction::m_transf3d0 ;
  const QPoint WrapperChemicalCmdToAvoAction::m_qpoint0(QPoint(0,0)) ;
  const double WrapperChemicalCmdToAvoAction::m_PI180=M_PI/180.0 ;
  const double WrapperChemicalCmdToAvoAction::m_180PI=180.0/M_PI ;


  WrapperChemicalCmdToAvoAction::WrapperChemicalCmdToAvoAction
    ( GLWidget *widget, WITD::ChemicalWrap *chemWrap, InputDevice::WmDevice *wmdev )

    : m_widget(widget), m_moleculeManip(NULL), 
      m_wmDev(wmdev),
      
      m_isCalculDistDiedre(false),
      m_isMoveAtom(false), m_tmpBarycenter(m_vect3d0),

      m_isRenderRect(false), m_rectP1(m_qpoint0), m_rectP2(m_qpoint0),

      m_isAtomDraw(false), m_isBondOrder(false),
      m_drawBeginAtom(false), m_drawCurrentAtom(false), m_drawBond(false),
      m_hasAddedBeginAtom(false), m_hasAddedCurAtom(false), m_hasAddedBond(false),
      m_beginPosDraw(m_vect3d0), m_curPosDraw(m_vect3d0),
      m_lastCursor(m_qpoint0),
      m_beginAtomDraw(NULL), m_curAtomDraw(NULL), m_bondDraw(NULL),
      m_timeFirst(0), m_timeSecond(0), m_canDrawOther(false)
  {
    /// Initiate the objects.
    m_moleculeManip = new MoleculeManipulation( widget->molecule() ) ;
    m_contextMenu = new ContextMenuToAvoAction( widget, m_moleculeManip, chemWrap ) ;
    m_transf3d0.matrix().setIdentity() ;

    /// Initiate some attributs to realize mouse simulations.
    m_testEventPress = false ;
    m_p = new QPoint(1,1) ;
    m_me1 = new QMouseEvent(QEvent::MouseButtonPress, *m_p, Qt::LeftButton, Qt::NoButton, Qt::NoModifier) ;
    m_me2 = new QMouseEvent(QEvent::MouseMove, *m_p, Qt::NoButton, Qt::LeftButton, Qt::NoModifier) ;
    m_me3 = new QMouseEvent(QEvent::MouseButtonRelease, *m_p, Qt::LeftButton, Qt::NoButton, Qt::NoModifier) ;

    m_time.start() ;
    m_nbUpdate1 = new WIWO<unsigned int>(20) ;
    m_nbUpdate2 = new WIWO<unsigned int>(20) ;
    m_nbUpdate3 = new WIWO<unsigned int>(20) ;
  }

  WrapperChemicalCmdToAvoAction::~WrapperChemicalCmdToAvoAction()
  {
    if( m_me1 != NULL ){ delete( m_me1 ) ; m_me1=NULL ; }
    if( m_me2 != NULL ){ delete( m_me2 ) ; m_me2=NULL ; }
    if( m_me3 != NULL ){ delete( m_me3 ) ; m_me3=NULL ; }
    if( m_p != NULL ){ delete( m_p ) ; m_p=NULL ; }

    if( m_nbUpdate1 != NULL ){ delete( m_nbUpdate1 ) ; m_nbUpdate1=NULL ; }
    if( m_nbUpdate2 != NULL ){ delete( m_nbUpdate2 ) ; m_nbUpdate2=NULL ; }
    if( m_nbUpdate3 != NULL ){ delete( m_nbUpdate3 ) ; m_nbUpdate3=NULL ; }
  }

  MoleculeManipulation* WrapperChemicalCmdToAvoAction::getMoleculeManip()
  { 
    return m_moleculeManip ; 
  }

  ContextMenuToAvoAction* WrapperChemicalCmdToAvoAction::getContextMenu()
  { 
    return m_contextMenu ; 
  }


  /**
    * Transform a wrapper action to an Avogadro action.
    * @param wmData All Wiimote data get.
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToAvoAction( WITD::ChemicalWrapData_from *data )
  {
    WITD::ChemicalWrapData_from::wrapperActions_t wa ;
    WITD::ChemicalWrapData_from::positionCamera_t pc ;
    WITD::ChemicalWrapData_from::positionPointed_t pp ;
    
    //bool upWa=data->getWrapperAction( wa ) ;
    //bool upPc=data->getPositionCamera( pc ) ;
    //bool upPp=data->getPositionPointed( pp ) ;

    wa = data->getWrapperAction() ;
    pc = data->getPositionCamera() ;
    pp = data->getPositionPointed() ;

    QPoint posCursor=pp.posCursor ;
    int state=wa.actionsWrapper ;
    Eigen::Vector3d pos3dCurrent=pp.pos3dCur ;
    Eigen::Vector3d pos3dLast=pp.pos3dLast ;
    double rotCamAxeXDeg=pc.angleRotateDegree[0] ;
    double rotCamAxeYDeg=pc.angleRotateDegree[1] ;
    double distCamXTranslate=pc.distanceTranslate[0] ;
    double distCamYTranslate=pc.distanceTranslate[1] ;
    double distCamZoom=pc.distanceTranslate[2] ;
    //cout << nbDotsDetected << " " << nbSourcesDetected << " " << distBetweenSource << endl ;

    // Adjustment for the rotation of atoms.
    if( WMAVO_IS2(state,WMAVO_CAM_ROTATE_BYWM) && m_widget->selectedPrimitives().size()>=2 )
    {
      WMAVO_SETOFF2( state, WMAVO_CAM_ROTATE_BYWM ) ;
      WMAVO_SETON2( state, WMAVO_ATOM_MOVE ) ;
      WMAVO_SETON2( state, WMAVO_ATOM_ROTATE ) ;
    }

    const Eigen::Vector3d pointRef=m_moleculeManip->getBarycenterMolecule() ;

    transformWrapperActionToMoveAtom( state, pointRef, pos3dCurrent, pos3dLast, rotCamAxeXDeg, rotCamAxeYDeg ) ;
    transformWrapperActionToMoveMouse( state, posCursor ) ;
    transformWrapperActionToSelectAtom( state, posCursor ) ;
    transformWrapperActionToCreateAtomBond( state, pointRef, posCursor ) ;
    transformWrapperActionToDeleteAllAtomBond( state ) ;
    transformWrapperActionToRemoveAtomBond( state, posCursor ) ;

    transformWrapperActionToRotateCamera( state, pointRef, rotCamAxeXDeg, rotCamAxeYDeg ) ;
    transformWrapperActionToTranslateCamera( state, pointRef, distCamXTranslate, distCamYTranslate ) ;
    transformWrapperActionToZoomCamera( state, pointRef, distCamZoom ) ;
    transformWrapperActionToInitiateCamera( state, pointRef ) ;

    transformWrapperActionToUseContextMenu( state, posCursor ) ;

    transformWrapperActionToAvoUpdate( state ) ;
  }


  void WrapperChemicalCmdToAvoAction::transformWrapperActionToAvoUpdate( int state )
  {
    //
    // Update Avogadro to see modification.

    updateForAvoActions1( state ) ;
    updateForAvoActions2( state ) ;
    updateForAvoActions3( state ) ;
  }


  /**
    * Transform a wrapper action to an Avogadro action : move the mouse cursor.
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The new position of the mouse cursor
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToMoveMouse( int wmavoAction, const QPoint& posCursor )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CURSOR_MOVE) || WMAVO_IS2(wmavoAction,WMAVO_CREATE) )
    {
      //cout << "WrapperChemicalCmdToAvoAction::transformWrapperActionToMoveMouse" << endl ;
      QCursor::setPos(posCursor) ;
    }
  }

  /**
    * Transform a wrapper action to an Avogadro action : select an atom.
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The position where a "click" is realised
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToSelectAtom( int wmavoAction, const QPoint& posCursor )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_SELECT) || WMAVO_IS2(wmavoAction,WMAVO_SELECT_MULTI) )
    {
      //cout << "WrapperChemicalCmdToAvoAction::transformWrapperActionToSelectAtom" << endl ;

      if( WMAVO_IS2(wmavoAction,WMAVO_SELECT) )
      { // Select only 1 object.

        // Just one rumble.
        InputDevice::RumbleSettings rumble(true, true, false, false, 10 ) ;
        rumble.setDistance( 0 ) ;
        InputDevice::WmDeviceData_to wmDevDataTo ;
        wmDevDataTo.setRumble( rumble ) ;
        m_wmDev->setDeviceDataTo( wmDevDataTo ) ;

        QPoint p=m_widget->mapFromGlobal(posCursor) ;
        QList<Primitive*> hitList ;
        Atom* atom=m_widget->computeClickedAtom( p ) ;
        bool hasAddHydrogen=m_moleculeManip->hasAddedHydrogen() ;
        
        // OR,
        // use the method below :
        #if 0
        QPoint p=m_widget->mapFromGlobal(posCursor) ;
        QList<GLHit> hits=m_widget->hits( p.x()-5, p.y()-5, 10, 10 ) ;
        QList<Primitive*> hitList ;

        foreach( const GLHit& hit, hits )
        {
          if( hit.type() == Primitive::AtomType )
          {
            //cout << "atom !!" << endl ;
            Atom *atom = m_widget->molecule()->atom( hit.name() ) ;
            hitList.append(atom) ;
          }
        }
        #endif

        if( atom == NULL )
        {
          m_widget->clearSelected() ;
        }
        else
        {
          if( m_isCalculDistDiedre )
          { // Manage the selection of atom. It works with an association of the WmTool class.

            // Put the "calcul distance" mode of the WrapperChemicalCmdToAvoAction class to off.
            m_isCalculDistDiedre = false ;

            // Inform the WmTool class of the selected atom.
            emit sendAtomToCalculDistDiedre( atom ) ; // To wmTool.

            // Nota Bene : it is the WmTool class which infoms the WrapperChemicalCmdToAvoAction class when
            // it is necessary to select an atom for the "calcul distance" mode.
          }
          else
          {
            hitList.append(atom) ;
            m_widget->toggleSelected(hitList) ;
            //m_widget->setSelected(hitList, true) ;

            // Select H-neighbors.
            if( hasAddHydrogen
                && m_widget->isSelected(static_cast<Primitive*>(atom))
                && !atom->isHydrogen() )
            {
              Atom *a=NULL ;
              PrimitiveList pl ;

              foreach( unsigned long ai, atom->neighbors() )
              {
                a = m_widget->molecule()->atomById(ai) ;

                if( a!=NULL && a->isHydrogen() )
                  pl.append( a ) ;
              }

              m_widget->setSelected( pl, true) ;
            }
          }
        }
      }
      else //if( WMAVO_IS2(wmavoAction,WMAVO_SELECT_MULTI) )
      { // Multiple selection.

        // For the display of the selection rectangle.
        if( !m_isRenderRect )
        { // Save the 1st point of the rectangle.

          m_isRenderRect = true ;
          m_rectP1 = posCursor ;
        }

        // Save the 2nd point of the rectangle.
        m_rectP2 = posCursor ;

        QPoint p1=m_widget->mapFromGlobal(m_rectP1) ;
        QPoint p2=m_widget->mapFromGlobal(m_rectP2) ;

        // Adjust the 1st point always at bottom/left,
        // the 2nd point always at up/right.
        int x1=( p1.x()<p2.x() ? p1.x() : p2.x() ) ;
        int y1=( p1.y()<p2.y() ? p1.y() : p2.y() ) ;

        int x2=( p1.x()>p2.x() ? p1.x() : p2.x() ) - 1 ;
        int y2=( p1.y()>p2.y() ? p1.y() : p2.y() ) - 1 ;

        // Inform the WmTool class of the 2 points of the selection rectangle for the display.
        emit renderedSelectRect( true, QPoint(x1,y1), QPoint(x2,y2) ) ;
      }
    }
    else
    {
      if( m_isRenderRect )
      {
        //
        // 1. Realize the selection

        QList<GLHit> hitList ;
        PrimitiveList pList ;
        Primitive *p=NULL ;
        Atom *a=NULL ;

        QPoint p1=m_widget->mapFromGlobal(m_rectP1) ;
        QPoint p2=m_widget->mapFromGlobal(m_rectP2) ;

        int x1=( p1.x()<p2.x() ? p1.x() : p2.x() ) ;
        int y1=( p1.y()<p2.y() ? p1.y() : p2.y() ) ;

        int x2=( p1.x()>p2.x() ? p1.x() : p2.x() ) ; // - 1 ;
        int y2=( p1.y()>p2.y() ? p1.y() : p2.y() ) ; // - 1 ;


        // Perform an OpenGL selection and retrieve the list of hits.
        hitList = m_widget->hits( x1, y1, x2-x1, y2-y1 ) ;

        if( hitList.empty() )
        {
          m_widget->clearSelected() ;
        }
        else
        {
          // Build a primitiveList for toggleSelected() method.
          foreach( const GLHit& hit, hitList )
          {
            if( hit.type() == Primitive::AtomType )
            {
              a = m_widget->molecule()->atom( hit.name() ) ;
              p = static_cast<Primitive *>( a ) ;

              if( p != NULL )
                pList.append( p ) ;
              else
                mytoolbox::dbgMsg( "Bug in WrapperChemicalCmdToAvoAction::transformWrapperActionToSelectAtom : a NULL-object not expected." ) ;
            }
          }

          // Toggle the selection.
          m_widget->toggleSelected( pList ) ; // or setSelected()
          //cout << "toggle primitive" << endl ;
        }

        //
        // 2. Finish the action.

        m_isRenderRect = false ;
        m_rectP1 = QPoint( 0, 0 ) ;
        m_rectP2 = QPoint( 0, 0 ) ;

        p1 = m_widget->mapFromGlobal(m_rectP1) ;
        p2 = m_widget->mapFromGlobal(m_rectP2) ;
        emit renderedSelectRect( false, p1, p2) ;
      }
    }
  }



  /**
    * Transform a wrapper action to an Avogadro action : move the selected atoms.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point
    * @param pos3dCurrent The current position calculate by the Wiimote
    * @param pos3dLast The last position calculate by the Wiimote
    * @param rotAtomDegX The desired X-axis angle
    * @param rotAtomDegY The desired Y-axis angle
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToMoveAtom
    ( int wmavoAction, 
      const Eigen::Vector3d& pointRef,
      const Eigen::Vector3d& pos3dCurrent, 
      const Eigen::Vector3d& pos3dLast, 
      double rotAtomDegX, 
      double rotAtomDegY )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_MOVE) )
    { // Object is in "travel mode", but just in the mode.
      // It is necessary to know what is the movement.

      if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_TRANSLATE) || WMAVO_IS2(wmavoAction,WMAVO_ATOM_ROTATE) )
      { // Work when an atom is moving. If no movement, do not pass here.

        //cout << "WrapperChemicalCmdToAvoAction::transformWrapperActionToMoveAtom" << endl ;

        Eigen::Vector3d vectAtomTranslate ;
        Eigen::Transform3d transfAtomRotate ;

        bool isMoved=calculateTransformationMatrix
                      ( wmavoAction, pos3dCurrent, pos3dLast, 
                        pointRef, 
                        rotAtomDegX, rotAtomDegY,
                        vectAtomTranslate, transfAtomRotate
                        ) ;

        if( isMoved )
        {
          if( !m_isMoveAtom )
            m_isMoveAtom = true ;

          QList<Primitive*> pList=m_widget->selectedPrimitives().subList(Primitive::AtomType) ;

          if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_TRANSLATE) )
            m_moleculeManip->tranlateAtomBegin( pList, vectAtomTranslate ) ;
          else if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_ROTATE) )
            m_moleculeManip->rotateAtomBegin( pList, transfAtomRotate ) ;
          
          // Active rumble in the Wiimote only if one atom is selected.
          if( pList.size() == 1 )
          {
            Atom *a=static_cast<Atom*>(pList.at(0)) ;

            if( a != NULL )
            {
              Atom *an=m_moleculeManip->calculateNearestAtom( a->pos(), a ) ;
              Eigen::Vector3d dist = *(a->pos()) - *(an->pos()) ;
              double act = dist.norm() ;

              InputDevice::RumbleSettings rumble(true, true, false, true ) ;
              rumble.setDistance( act ) ;
              InputDevice::WmDeviceData_to wmDevDataTo ;
              wmDevDataTo.setRumble( rumble ) ;
              m_wmDev->setDeviceDataTo( wmDevDataTo ) ;
            }
          }
        }
      }
    }
    else
    { // Finish the action.

      if( m_isMoveAtom ) // Caution, this attribut is initialised/used in moveAtom*() methods.
      {
        m_isMoveAtom = false ;

        QList<Primitive*> pList=m_widget->selectedPrimitives().subList(Primitive::AtomType) ;
        m_moleculeManip->moveAtomEnd( pList ) ;

        InputDevice::RumbleSettings rumble(true, false, false, false ) ;
        rumble.setDistance( 0 ) ;
        InputDevice::WmDeviceData_to wmDevDataTo ;
        wmDevDataTo.setRumble( rumble ) ;
        m_wmDev->setDeviceDataTo( wmDevDataTo ) ;

        m_tmpBarycenter = m_vect3d0 ;
      }
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : create atom(s) and bond(s).
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point
    * @param posCursor The position of the cursor
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToCreateAtomBond
     ( int wmavoAction, const Eigen::Vector3d& pointRef, const QPoint &posCursor )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CREATE) || m_isAtomDraw )
      // for m_isAtomDraw : Necessary for the action of "isCreateAtom".
    {
      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::transformWrapperActionToCreateAtomBond" ;

      Molecule *mol=m_widget->molecule() ;

      // molecule->lock()->tryLockForWrite() :
      // GLWidget::render(): Could not get read lock on molecule.
      // Lock the write on the molecule => so it locks the render too ...
      // This means the possibility that the Avogadro software can be really multi-thread
      // (or in anticipation).
      // The lock justifies it by the array used to store a new atom, and with the search
      // of a new id for this new atom y tutti quanti ...
      //if( molecule->lock()->tryLockForWrite() )
      //{

        // Add an atom.

        QPoint p=m_widget->mapFromGlobal(posCursor) ;

        if( !m_isAtomDraw )
        { // The 1st action : a request of creation

          m_lastCursor = p ;
          Primitive* prim=m_widget->computeClickedPrimitive( p ) ;
          m_timeFirst = m_time.elapsed() ;

          if( prim == NULL )
          {
            m_isAtomDraw = true ;
            m_drawBeginAtom = true ;
            m_drawCurrentAtom = false ;
            m_drawBond = false ;

            //cout << "cursor position:" << p.x() << "," << p.y() << endl ;
            //cout << "ref position:" << pointRef[0] << "," << pointRef[1] << "," << pointRef[2] << endl ;
            m_beginPosDraw = m_widget->camera()->unProject( p, pointRef ) ;
            m_curPosDraw = m_beginPosDraw ;
          }
          else if( prim->type() == Primitive::AtomType )
          {
            m_isAtomDraw = true ;
            m_drawBeginAtom = false ;
            m_drawCurrentAtom = false ;
            m_drawBond = false ;

            m_beginAtomDraw = static_cast<Atom*>(prim) ;
            m_beginPosDraw = *(m_beginAtomDraw->pos()) ;
            m_curPosDraw = m_beginPosDraw ;
          }
          else if( prim->type() == Primitive::BondType )
          {
            m_isBondOrder = true ;
            m_isAtomDraw = true ;
            m_drawBeginAtom = false ;
            m_drawCurrentAtom = false ;
            m_drawBond = false ;

            m_bondDraw = static_cast<Bond*>(prim) ;
          }
          //else
            // Nothing to do.

          // Request the temporary display of the 1st atom(by the WmTool class).
          if( m_isAtomDraw || m_isBondOrder )
            emit renderedAtomBond( m_beginPosDraw, m_curPosDraw, m_drawBeginAtom, m_drawCurrentAtom, m_drawBond ) ;
        }
        else if( m_isAtomDraw && WMAVO_IS2(wmavoAction,WMAVO_CREATE) && !m_isBondOrder )
        { // The 2nd action.
          // That means, the 1st atom has been "selected/created" and
          // the mouse is moving to create/select an 2nd atom with (new) bond.


          // Timeout before to create a 2nd atom.
          if( !m_canDrawOther )
          {
            m_timeSecond = m_time.elapsed() ;

            if( (m_timeSecond-m_timeFirst) > 1000 )
              m_canDrawOther = true ;
          }

          if( m_canDrawOther )
          {
            m_curPosDraw = m_widget->camera()->unProject(p, pointRef) ;

            Atom *an = m_moleculeManip->calculateNearestAtom( &m_curPosDraw, NULL ) ;

            if( an != NULL )
            {
              Eigen::Vector3d dist=m_curPosDraw - *(an->pos()) ;
              double act=dist.norm() ;

              // Activate rumble.
              InputDevice::RumbleSettings rumble(true, true, false, true ) ;
              rumble.setDistance( act ) ;
              InputDevice::WmDeviceData_to wmDevDataTo ;
              wmDevDataTo.setRumble( rumble ) ;
              m_wmDev->setDeviceDataTo( wmDevDataTo ) ;
            }

            // Methode0 : SPEED !!
            Atom* a=NULL ;

            // Methode1 : SLOW !!
            //a = m_widget->computeClickedAtom( p ) ;

            // Methode2 : SLOW !!
            /*
            QList<GLHit> hits=m_widget->hits( p.x()-5, p.y()-5, 10, 10 ) ;

            foreach( const GLHit& hit, hits )
            {
              if( hit.type() == Primitive::AtomType )
              { // Le 1er element est le plus proche.

                a = m_widget->molecule()->atom( hit.name() ) ;
                break ;
              }
            }*/

            if( a == NULL )
            {
              double var1=m_beginPosDraw[0]-m_curPosDraw[0] ;
              double var2=m_beginPosDraw[1]-m_curPosDraw[1] ;
              double var3=m_beginPosDraw[2]-m_curPosDraw[2] ;
              double distVect=sqrt( var1*var1 + var2*var2 + var3*var3 ) ;

              // Draw a 2nd atom only if the distance is bigger than ...
              if( distVect > WMEX_DISTBEFORE_CREATE )
              {
                //cout << "Display current atom & bond" << endl ;
                m_drawCurrentAtom = true ;

                if( m_beginAtomDraw!=NULL && m_beginAtomDraw->isHydrogen()
                    && m_beginAtomDraw->bonds().count()>0 )
                  m_drawBond = false ;
                else
                  m_drawBond = true ;

                m_curAtomDraw = a ;
              }
              else
              {
                m_drawCurrentAtom = false ;
                m_drawBond = false ;

                m_curAtomDraw = NULL ;
              }
            }
            else if( *(a->pos()) == m_beginPosDraw )
            {
              //cout << "Display nothing" << endl ;

              m_drawCurrentAtom = false ;
              m_drawBond = false ;

              m_curAtomDraw = NULL ;
            }
            else //if( a )
            {
              //cout << "Display Bond" << endl ;

              m_drawCurrentAtom = false ;
              m_drawBond = true ;

              // Limit the number of bond if Hydrogen Atom.
              if( a->isHydrogen() && a->bonds().count() > 0 )
                m_drawBond = false ;

              if( m_drawBond && m_beginAtomDraw!=NULL
                  && m_beginAtomDraw->isHydrogen() && m_beginAtomDraw->bonds().count() > 0 )
                m_drawBond = false ;

              m_curAtomDraw = a ;
              m_curPosDraw = *(a->pos()) ;
            }


            // Request the temporary display of the atoms and bond (by the WmTool class).
            emit renderedAtomBond( m_beginPosDraw, m_curPosDraw, m_drawBeginAtom, m_drawCurrentAtom, m_drawBond ) ;
          }
        }
        else if( m_isAtomDraw && !WMAVO_IS2(wmavoAction,WMAVO_CREATE) )
        { // The 3rd and last action : creation
          // - either adjust number of bond ;
          // - or create atoms(s) and bond.

          bool addSmth=false ;
          //QUndoCommand *undo=NULL ;

          if( m_isBondOrder )
          {
            //int oldBondOrder = m_bondDraw->order() ;

            #if !AVO_DEPRECATED_FCT
            // 1.
            m_moleculeManip->changeOrderBondBy1( m_bondDraw ) ;

            #else
            // 2.
            undo = new ChangeBondOrderDrawCommand( m_bondDraw, oldBondOrder, m_addHydrogens ) ;
            m_widget->undoStack()->push( undo ) ;
            #endif

          }
          else //if( m_isAtomDraw && !m_isBondOrder )
          {
            //cout << "End of the creation of atom/bond" << endl ;
            Atom* a=NULL ;
            Eigen::Vector3d addAtomPos ;

            InputDevice::RumbleSettings rumble(true, false, false, false ) ;
            rumble.setDistance( 0 ) ;
            InputDevice::WmDeviceData_to wmDevDataTo ;
            wmDevDataTo.setRumble( rumble ) ;
            m_wmDev->setDeviceDataTo( wmDevDataTo ) ;

            if( m_beginAtomDraw == NULL )
            {
              addSmth = true ;
              m_hasAddedBeginAtom = true ;
            }

            // Timeout before to create a 2nd atom.
            if( !m_canDrawOther )
            {
              m_timeSecond = m_time.elapsed() ;

              if( (m_timeSecond-m_timeFirst) > 1000 )
                m_canDrawOther = true ;
            }

            // Add 2nd atom & bond.
            if( m_canDrawOther )
            {
              a = m_widget->computeClickedAtom( p ) ;

              if( a == NULL )
              { // Create atome/bond.

                double var1=m_beginPosDraw[0]-m_curPosDraw[0] ;
                double var2=m_beginPosDraw[1]-m_curPosDraw[1] ;
                double var3=m_beginPosDraw[2]-m_curPosDraw[2] ;
                double distVect=sqrt( var1*var1 + var2*var2 + var3*var3 ) ;

                // Draw a 2nd atom only if the distance is bigger than ...
                if( distVect > 0.6 )
                {
                  addAtomPos = m_widget->camera()->unProject( p, pointRef ) ;

                  addSmth = true ;
                  m_hasAddedCurAtom = true ;

                  if( m_drawBond
                      /* !(m_curAtomDraw->isHydrogen() && m_curAtomDraw->bonds().count()>0)
                         && !(m_beginAtomDraw->isHydrogen() && m_beginAtomDraw->bonds().count()>0)
                      */
                    )
                    m_hasAddedBond = true ;
                }
              }
              else
              { // Create bond.

                if( *(a->pos()) != m_beginPosDraw
                    && mol->bond(a,m_beginAtomDraw) == NULL
                    && m_drawBond
                       /* !(a->isHydrogen() && a->bonds().count()>0)
                          && !(m_beginAtomDraw->isHydrogen() && m_beginAtomDraw->bonds().count()>0)
                       */
                  )
                {
                  m_curAtomDraw = a ;
                  addSmth = true ;
                  m_hasAddedBond = true ;
                }
              }
            }


            if( mol->lock()->tryLockForWrite() )
            {
              int atomicNumber=m_moleculeManip->getAtomicNumberCurrent() ;

              // Create just the 1st atom.
              if( m_hasAddedBeginAtom && !m_hasAddedCurAtom && !m_hasAddedBond )
                m_beginAtomDraw = m_moleculeManip->addAtom( &m_beginPosDraw, atomicNumber ) ;

              // Create just the 2nd atom.
              if( !m_hasAddedBeginAtom && m_hasAddedCurAtom && !m_hasAddedBond )
                m_curAtomDraw = m_moleculeManip->addAtom( &addAtomPos, atomicNumber ) ;

              // Create just the bond.
              if( !m_hasAddedBeginAtom && !m_hasAddedCurAtom && m_hasAddedBond )
                m_bondDraw = m_moleculeManip->addBond( m_beginAtomDraw, a, 1 ) ;

              // Create the 2nd atom bonded at 1st.
              if( !m_hasAddedBeginAtom && m_hasAddedCurAtom && m_hasAddedBond )
                m_curAtomDraw = m_moleculeManip->addAtom
                                                ( &addAtomPos, atomicNumber,
                                                  m_beginAtomDraw, 1 ) ;


              // Create the 1st atom bonded at 2nd.
              if( m_hasAddedBeginAtom && !m_hasAddedCurAtom && m_hasAddedBond )
                m_beginAtomDraw = m_moleculeManip->addAtom
                                                  ( &m_beginPosDraw, atomicNumber,
                                                    m_curAtomDraw, 1 ) ;

              // Create 2 atoms.
              if( m_hasAddedBeginAtom && m_hasAddedCurAtom )
              {
                int order=0 ;
                PrimitiveList *pl=NULL ;

                if( m_hasAddedBond )
                  order = 1 ;

                pl = m_moleculeManip->addAtoms
                                     ( &m_beginPosDraw, atomicNumber,
                                       &addAtomPos, atomicNumber, order ) ;

                if( pl!=NULL && pl->size()>=2 )
                {
                  PrimitiveList::const_iterator ipl=pl->begin() ;
                  m_beginAtomDraw = static_cast<Atom*>(*ipl) ;
                  ipl++ ;
                  m_curAtomDraw = static_cast<Atom*>(*ipl) ;
                }

                if( pl != NULL )
                  delete pl ;
              }


              // Substitute atom by atom.
              if( !m_hasAddedBeginAtom && !m_hasAddedCurAtom && !m_hasAddedBond && !m_canDrawOther )
                m_moleculeManip->changeAtomicNumber( m_beginAtomDraw, atomicNumber ) ;



              GLfloat projectionMatrix[16] ;
              glGetFloatv( GL_PROJECTION_MATRIX, projectionMatrix ) ;

              //cout << "Projection Matrix:" << endl ;
              //cout<<" "<<projectionMatrix[0]<<" "<<projectionMatrix[4]<<" "<<projectionMatrix[8]<<" "<<projectionMatrix[12]<<endl;
              //cout<<" "<<projectionMatrix[1]<<" "<<projectionMatrix[5]<<" "<<projectionMatrix[9]<<" "<<projectionMatrix[13]<<endl;
              //cout<<" "<<projectionMatrix[2]<<" "<<projectionMatrix[6]<<" "<<projectionMatrix[10]<<" "<<projectionMatrix[14]<<endl;
              //cout<<" "<<projectionMatrix[3]<<" "<<projectionMatrix[7]<<" "<<projectionMatrix[11]<<" "<<projectionMatrix[15]<<endl;


            }

            mol->lock()->unlock() ;
            mol->update() ;
          }

          if( addSmth )
          {
            InputDevice::RumbleSettings rumble(true, true, false, false, 10 ) ;
            rumble.setDistance( 0 ) ;
            InputDevice::WmDeviceData_to wmDevDataTo ;
            wmDevDataTo.setRumble( rumble ) ;
            m_wmDev->setDeviceDataTo( wmDevDataTo ) ;
          }

          //addAdjustHydrogenRedoUndo( molecule ) ;


          // Initialization before next use.
          m_isBondOrder=false ; m_isAtomDraw=false ;
          m_drawBeginAtom=false ; m_drawCurrentAtom=false ; m_drawBond=false ;
          m_hasAddedBeginAtom=false ; m_hasAddedCurAtom=false ; m_hasAddedBond=false ;
          m_beginAtomDraw=NULL ; m_curAtomDraw=NULL ; m_bondDraw=NULL ;

          m_timeFirst=0 ; m_timeSecond=0 ;
          m_canDrawOther = false ;

          // "Push" all modifications & redraw of the molecule.
          emit renderedAtomBond( m_vect3d0, m_vect3d0, false, false, false ) ;
        }
      //}
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : delete all atom.
    * @param wmavoAction All actions ask by the wrapper
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToDeleteAllAtomBond( int wmavoAction )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_DELETEALL) )
    {
      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::transformWrapperActionToDeleteAllAtomBond" ;
      m_moleculeManip->deleteAllElement() ;
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : delete atom(s).
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The position of the cursor
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToRemoveAtomBond( int wmavoAction, const QPoint &posCursor )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_DELETE) )
    {
      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::transformWrapperActionToRemoveAtomBond" ;

      Molecule *mol=m_widget->molecule() ;

      if( mol->lock()->tryLockForWrite() )
      {        
        QPoint p=m_widget->mapFromGlobal(posCursor) ;
        Primitive* prim=m_widget->computeClickedPrimitive( p ) ;
        PrimitiveList pl=m_widget->selectedPrimitives() ;

        if( prim == NULL )
        { // Remove the selected atoms.

          #if !AVO_DEPRECATED_FCT
          m_moleculeManip->removeAtoms( &pl ) ;

          #else
          // 2. with undo/redo, not adjust hydrogen ...
          deleteSelectedElementUndoRedo( mol ) ;
          #endif
        }
        else
        {

          if( prim->type() == Primitive::AtomType )
          { // Remove atom.

            Atom *atom = static_cast<Atom*>(prim) ;
            m_moleculeManip->removeAtom( atom ) ;
          }

          if( prim->type() == Primitive::BondType )
          { // Remove bond.

            Bond *bond = static_cast<Bond*>(prim) ;

            #if !AVO_DEPRECATED_FCT
            // 1.
            m_moleculeManip->removeBond( bond ) ;
            
            #else
            // 2.
            deleteBondWithUndoRedo( bond ) ;
            #endif
          }
        }

        mol->lock()->unlock() ;
      }
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : rotate the camera.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point
    * @param rotCamAxeXDeg The desired angle on the X-axis of the screen
    * @param rotCamAxeYDeg The desired angle on the Y-axis of the screen
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToRotateCamera
        ( int wmavoAction,
          const Eigen::Vector3d &pointRef,
          double rotCamAxeXDeg, double rotCamAxeYDeg )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE) )
    {
      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::transformWrapperActionToRotateCamera" ;

      if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE_BYWM) )
      { // If use the cross of the Wiimote.

        // Use this method to get the wanted angle of rotation.
        // Value in (radian / Avogadro::ROTATION_SPEED) == the desired angle.
        double rotCamAxeXRad = (rotCamAxeXDeg*m_PI180) / Avogadro::ROTATION_SPEED ;
        double rotCamAxeYRad = (rotCamAxeYDeg*m_PI180) / Avogadro::ROTATION_SPEED ;

        Navigate::rotate( m_widget, pointRef, rotCamAxeXRad, rotCamAxeYRad ) ;
      }
      else if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE_BYNC) )
      { // Turn in a direction.
        // Do not search an unit speed or other, just a direction.

        Navigate::rotate( m_widget, pointRef, rotCamAxeXDeg, rotCamAxeYDeg ) ;
      }
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : translate the camera.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point
    * @param distCamXTranslate Desired distance on the X-axis of the screen
    * @param distCamYTranslate Desired distance on the Y-axis of the screen
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToTranslateCamera
    ( int wmavoAction, const Eigen::Vector3d &pointRef, double distCamXTranslate, double distCamYTranslate )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CAM_TRANSLATE) )
    {
      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::transformWrapperActionToTranslateCamera" ;
      Navigate::translate( m_widget, pointRef, distCamXTranslate, distCamYTranslate ) ;
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : zoom the camera.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point
    * @param distCamZoom Desired distance for the zoom on the Z-axis of the screen
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToZoomCamera
    ( int wmavoAction, const Eigen::Vector3d &pointRef, double distCamZoom )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CAM_ZOOM) )
    {
      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::transformWrapperActionToZoomCamera" ;
      Navigate::zoom( m_widget, pointRef, distCamZoom ) ;
    }
  }


  /**
    * Transform a wrapper action to an Avogadro action : initiate the camera.
    * @param wmavoAction All actions ask by the wrapper
    * @param pointRef The position of the reference point.
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToInitiateCamera
    ( int wmavoAction, const Eigen::Vector3d &pointRef )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_CAM_INITIAT) )
    {
      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::transformWrapperActionToInitiateCamera" ;

      #if 0
      // 1
      //m_widget->camera()->setModelview( m_cameraInitialViewPoint ) ;
      if( !(
            ((rotCamAxeXDeg==90.0 || rotCamAxeXDeg==-90.0) && rotCamAxeYDeg==0.0)
            || ((rotCamAxeYDeg==90.0 || rotCamAxeYDeg==-90.0) && rotCamAxeXDeg==0.0)
           )
        )
      { // If not use the cross of the Wiimote.

      // 2
      //mytoolbox::dbgMsg( "pointRefRot:" << pointRefRot[0] << pointRefRot[1] << pointRefRot[2] ;
      //Navigate::translate( m_widget, pointRefRot, -pointRefRot[0], -pointRefRot[1] ) ;

      // 3
      Eigen::Vector3d barycenterScreen=m_widget->camera()->project(pointRefRot) ;
      QPoint barycenterScreen2(barycenterScreen[0], barycenterScreen[1]) ;
      //mytoolbox::dbgMsg( "pointRefRot:" << barycenterScreen[0] << barycenterScreen[1] << barycenterScreen[2] ;
      //Navigate::translate( m_widget, pointRefRot, -barycentreEcran[0], -barycentreEcran[1] ) ;

      // 4
      //Navigate::zoom( m_widget, pointRefRot, m_widget->cam_beginPosDrawmera()->distance(pointRefRot)/*+10*/ ) ;
      //mytoolbox::dbgMsg( "   distance:" << m_widget->camera()->distance(pointRefRot) ;

      // 5
      Eigen::Vector3d transformedGoal = m_widget->camera()->modelview() * pointRefRot ;
      double distance=m_widget->camera()->distance(pointRefRot) ;
      double distanceToGoal = transformedGoal.norm() ;

      //mytoolbox::dbgMsg( " distance:" << distance ;
      //mytoolbox::dbgMsg( " distanceToGoal:" << distanceToGoal ;

      /*
      double distanceToGoal = transformedGoal.norm();m_beginPosDraw
      double t = ZOOM_SPEED * delta;
      const double minDistanceToGoal = 2.0 * CAMERA_NEAR_DISTANCE;
      double u = minDistanceToGoal / distanceToGoal - 1.0;
      if( t < u )
      {
        t = u;
        Navigate::rotate( m_widget, pointRefRot, rotCamAxeXDeg, rotCamAxeYDeg ) ;
               }
      widget->camera()->modelview().pretranslate(transformedGoal * t);
      */

      // 6
      //m_widget->camera()->modelview().pretranslate(-transformedGoal /*+ (camBackTransformedZAxis*-40)*/ ) ;
      //m_widget->camera()->modelview().translate(-transformedGoal /*+ (camBackTransformedZAxis*-10)*/ ) ;

      // 7
      //Eigen::Vector3d camBackTransformedZAxis=m_widget->camera()->transformedZAxis() ;
      //m_widget->camera()->modelview().translate( camBackTransformedZAxis*-10.0 ) ;


      distance=m_widget->camera()->distance(pointRefRot) ;
      distanceToGoal = transformedGoal.norm();

      //mytoolbox::dbgMsg( " distance:" << distance ;
      //mytoolbox::dbgMsg( " distanceToGoal:" << distanceToGoal ;
      #endif



      //Eigen::Vector3d barycenterScreen=m_widget->camera()->project(pointRefRot) ;
      //QPoint barycenterScreen2(barycenterScreen[0], barycenterScreen[1]) ;
      //mytoolbox::dbgMsg( "pointRefRot:" << barycenterScreen[0] << barycenterScreen[1] << barycenterScreen[2] ;

      //Eigen::Transform3d cam=m_widget->camera()->modelview() ;

      /* OK
      Eigen::Vector3d right(cam(0,0), cam(1,0), cam(2,0)) ;
      Eigen::Vector3d up(cam(0,1), cam(1,1), cam(2,1)) ;
      Eigen::Vector3d dir(cam(0,2), cam(1,2), cam(2,2)) ;
      Eigen::Vector3d pos(cam(0,3), cam(1,3), cam(2,3)) ;

      cout << "right:" << right << endl ;
      cout << "dir:" << dir << endl  ;
      cout << "up:" << up << endl  ;
      cout << "pos:" << pos << endl  ;
      */

      /* OK
      cam(0,3) = 0 ;
      cam(1,3) = 0 ;
      cam(2,3) = -20 ;
      */

      /* Oui, et non, apres quelques rotations de camera, le barycentre n'est plus centre.
      mytoolbox::dbgMsg( "pointRefRot:" << pointRefRot[0] << pointRefRot[1] ;
      cam(0,3) = -pointRefRot[0] ;
      cam(1,3) = -pointRefRot[1] ;
      cam(2,3) = -25 ;

      m_widget->camera()->setModelview(cam) ;
      */

      Eigen::Vector3d barycenterScreen=m_widget->camera()->project( pointRef ) ;
      QPoint barycenterScreen2((int)barycenterScreen[0], (int)barycenterScreen[1]) ;

      GLint params[4] ;

      // Do not work (with .h and compilation flag, the final error is : ~"impossible to use without a first call of glinit"~).
      //int screen_pos_x = glutGet((GLenum)GLUT_WINDOW_X);
      //int screen_pos_y = glutGet((GLenum)GLUT_WINDOW_Y);
      //mytoolbox::dbgMsg( "  :" << screen_pos_x << screen_pos_y ;

      glGetIntegerv( GL_VIEWPORT, params ) ;

      GLenum errCode ;
      const GLubyte *errString ;
      if( (errCode=glGetError()) != GL_NO_ERROR )
      {
        errString = gluErrorString( errCode ) ;
        fprintf (stderr, "OpenGL Error: %s\n", errString);
      }

      GLdouble x=params[0] ;
      GLdouble y=params[1] ;
      GLdouble width=params[2] ;
      GLdouble height=params[3] ;

      QPoint widgetCenter( (int)((x+width)/2.0), (int)((y+height)/2.0) ) ;

      Navigate::translate( m_widget, pointRef, barycenterScreen2, widgetCenter ) ;


      Eigen::Transform3d cam=m_widget->camera()->modelview() ;
      cam(2,3) = -25 ;
      m_widget->camera()->setModelview(cam) ;

    }
  }

  
  /**
    * Transform a wrapper action to an context menu action.
    * @param wmavoAction All actions ask by the wrapper
    * @param posCursor The position of the cursor
    */
  void WrapperChemicalCmdToAvoAction::transformWrapperActionToUseContextMenu( int &wmavoAction, const QPoint &posCursor )
  {
    m_contextMenu->manageAction( wmavoAction, posCursor ) ;
  }
  

  /**
    * Update Avogadro according to the Avogadro actions realized.
    * Here, the update is for the Avogadro delete actions.
    * @param wmavoAction All actions ask by the wrapper
    */
  void WrapperChemicalCmdToAvoAction::updateForAvoActions1( int wmavoAction )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_DELETE)
        || WMAVO_IS2(wmavoAction,WMAVO_DELETEALL)
        /*|| WMAVO_IS2(wmavoAction,WMAVO_CREATE)*/
        // Put in the transformWrapperActionToCreateAtomBond() method to gain update.
      )
    {
      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::updateForAvoActions1" ;

      // Update
      // If we have done stuff then trigger a redraw of the molecule
      m_widget->molecule()->update() ;

      /*
      // Not resolve an update problem ...
      m_widget->molecule()->update() ;
      m_widget->update() ; // update( &Region ), update( int, int, int, int ) ...
      m_widget->updateGeometry() ;
      m_widget->updateGL() ;
      m_widget->updateOverlayGL() ;
      m_widget->updatesEnabled() ;

      //m_widget->molecule()->updateAtom() ;
      m_widget->molecule()->updateMolecule() ;
      m_widget->molecule()->calculateGroupIndices() ;
      */
    }
  }


  /**
    * Update Avogadro according to the Avogadro actions realized.
    * Here, the update is for a lot of Avogadro actions.
    * This is a special update, because it simulates a mouse click to realize update.
    * In fact, some optimization are available only when some Avogadro class realize
    * update.
    * <br/>To activate the quick render (the previous optimization) , it is necessary to simulate a mouse click.
    * Explanation, the quick render is activated when :
    * - Check the quick render option
    *      Set (allowQuickRender) attribut to enable. Now Avogadro MAY accept
    *      quick render.
    * - While a mouse movement, if the mouse is down
    *      The call of GLWidget::mouseMoveEvent(), and only this method sets
    *      the (quickRender) attribut at true.
    *
    * @param wmavoAction All actions ask by the wrapper
    */
  void WrapperChemicalCmdToAvoAction::updateForAvoActions2( int wmavoAction )
  {
    if( //(WMAVO_IS2(wmavoAction, WMAVO_MENU_ACTIVE) ? 0 // To update wmInfo in wmTool class
        //                                           : 1 ) // To avoid a bug with periodic table.
        //|| 
        WMAVO_IS2(wmavoAction,WMAVO_SELECT)
        || WMAVO_IS2(wmavoAction,WMAVO_SELECT_MULTI)
        || WMAVO_IS2(wmavoAction,WMAVO_CREATE)
        || WMAVO_IS2(wmavoAction,WMAVO_CAM_ROTATE)
        || WMAVO_IS2(wmavoAction,WMAVO_CAM_ZOOM)
        || WMAVO_IS2(wmavoAction,WMAVO_CAM_TRANSLATE)
        || WMAVO_IS2(wmavoAction,WMAVO_CAM_INITIAT)
      )
    {

      //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::updateForAvoActions2" ;


      if( !m_widget->quickRender() )
      {
        // 1. No quick render.
        m_widget->update() ;
      }
      else
      {
        // 2. No compile : mouseMove is protected.
        // Call directly GLWidget->mouseMove signal.
        //emit m_widget->mouseMove(&me) ;

        // 3. Call directly Tool->mouseMouseEvent. No quick render.
        //m_widget->tool()->mouseMoveEvent( m_widget, &me) ;
        //m_widget->tool()->mouseMoveEvent( m_widget->current(), &me) ;

        // 4. Try Fake mouse event. WORKS !!!
        if( !m_testEventPress )
        {
          m_testEventPress = true ;
          QApplication::sendEvent( m_widget->m_current, m_me1 ) ;
        }

        QApplication::sendEvent( m_widget->m_current, m_me2 ) ;
        

        // Install something else.
        // 5. Try Fake mouse event.
        //qTestEventList events;
        //events.addMouseMove(p,1);
        //events.simulate(m_widget);
      }
    }
    else
    {
      // 4. Finish fake mouse event.
      if( m_widget->quickRender() && m_testEventPress )
      {
        m_testEventPress = false ;
        QApplication::sendEvent( m_widget->m_current, m_me3 ) ;
      }
    }
  }


  /**
    * Update Avogadro according to the Avogadro actions realized.
    * Here, the update is for the Avogadro move actions.
    * @param wmavoAction All actions ask by the wrapper
    */
  void WrapperChemicalCmdToAvoAction::updateForAvoActions3( int wmavoAction )
  {
    if( WMAVO_IS2(wmavoAction,WMAVO_ATOM_MOVE) )
    { // Object is in "travel mode", but just in the mode.
      // It is necessary to know what is the movement.

      if( (WMAVO_IS2(wmavoAction,WMAVO_ATOM_TRANSLATE) || WMAVO_IS2(wmavoAction,WMAVO_ATOM_ROTATE))
          && m_widget->selectedPrimitives().size()>0
        )
      {
        //mytoolbox::dbgMsg( "WrapperChemicalCmdToAvoAction::updateForAvoActions3" ;

        // 1. No Quick Render.
        //m_widget->molecule()->update() ; // Update & Redraw (without quick Render)

        // 2. Quick Render seems activated, but it lags ...
        m_widget->molecule()->updateMolecule() ; // Update & Redraw.
      }
    }
  }


    /**
    * Calculate the transformation vector and/or matrix according to the need.
    * "Convert" the wiimote coordinate system to the Avogadro coordinate system.
    * @return TRUE if the transformation matrix is different to null ; FALSE else.
    * @param wmactions All actions ask by the wrapper
    * @param curPos The current position calculate by the Wiimote
    * @param lastPos The last position calculate by the Wiimote
    * @param refPoint_in The position of the reference point.
    * @param rotAtomdegX_in The desired X-axis angle
    * @param rotAtomdegY_in The desired Y-axis angle
    */
  bool WrapperChemicalCmdToAvoAction::calculateTransformationMatrix
        ( int wmactions_in, 
          const Eigen::Vector3d& curPos_in, 
          const Eigen::Vector3d& lastPos_in, 
          const Eigen::Vector3d& refPoint_in, 
          double rotAtomdegX_in, 
          double rotAtomdegY_in,
          Eigen::Vector3d &vectAtomTranslate_out, 
          Eigen::Transform3d &transfAtomRotate_out )
  {
    bool isMoved=false ;

    if( WMAVO_IS2(wmactions_in,WMAVO_ATOM_TRANSLATE) || WMAVO_IS2(wmactions_in,WMAVO_ATOM_ROTATE) )
    {
      QPoint currentPoint((int)curPos_in[0], (int)curPos_in[1]) ;
      QPoint lastPoint((int)lastPos_in[0], (int)lastPos_in[1]) ;

      Eigen::Vector3d fromPos=m_widget->camera()->unProject( lastPoint, refPoint_in ) ;
      Eigen::Vector3d toPos=m_widget->camera()->unProject( currentPoint, refPoint_in ) ;
      Eigen::Vector3d camBackTransformedXAxis=m_widget->camera()->backTransformedXAxis() ;
      Eigen::Vector3d camBackTransformedYAxis=m_widget->camera()->backTransformedYAxis() ;
      Eigen::Vector3d camBackTransformedZAxis=m_widget->camera()->backTransformedZAxis() ;

      if( WMAVO_IS2(wmactions_in,WMAVO_ATOM_TRANSLATE)
          && !(curPos_in[0]==lastPos_in[0] && curPos_in[1]==lastPos_in[1]) )
      {
        vectAtomTranslate_out = (toPos - fromPos) / WMAVO_ATOM_SMOOTHED_MOVE_XY  ;

        //cout << "currentWmPos.x():" << currentWmPos.x() << " currentWmPos.y():" << currentWmPos.y() << endl ;
        //cout << "   lastWmPos.x():" << lastWmPos.x() << " lastWmPos.y():" << lastWmPos.y() << endl ;
        //cout << "  fromPos[0]:" << fromPos[0] << " fromPos[1]:" << fromPos[1] << " fromPos[2]:" << fromPos[2] << endl ;
        //cout << "    toPos[0]:" << toPos[0] << " toPos[1]:" << toPos[1] << " fromPos[2]:" << fromPos[2] << endl ;
        //cout << "      newVectAtomTranslate:" << m_vectAtomTranslate[0] << " " << m_vectAtomTranslate[1] << " " << m_vectAtomTranslate[2] << endl ;

        if( WMAVO_IS2(wmactions_in,WMAVO_ATOM_TRANSLATE) )
        {
          // Z-movement.
          if( (curPos_in[2]-lastPos_in[2]) <= -WMAVO_WM_Z_MINPOINTING_MOVEALLOWED )
            vectAtomTranslate_out += (camBackTransformedZAxis*WMAVO_ATOM_MAX_MOVE_Z) ;
          if( (curPos_in[2]-lastPos_in[2]) >= WMAVO_WM_Z_MINPOINTING_MOVEALLOWED )
            vectAtomTranslate_out -= (camBackTransformedZAxis*WMAVO_ATOM_MAX_MOVE_Z) ;
        }

        isMoved = true ;
        //cout << "      m_vectAtomTranslate:" << m_vectAtomTranslate[0] << " " << m_vectAtomTranslate[1] << " " << m_vectAtomTranslate[2] << endl ;
      }
      else if( WMAVO_IS2(wmactions_in,WMAVO_ATOM_ROTATE) )
      {
        if( m_tmpBarycenter == m_vect3d0 )
        { // Calculate the barycenter of selected atoms.

          Eigen::Vector3d tmp=m_vect3d0 ;
          int i=0 ;
          Atom *a=NULL ;

          foreach( Primitive *p, m_widget->selectedPrimitives() )
          {
            if( p->type() == Primitive::AtomType )
            {
              a = static_cast<Atom*>(p) ;
              tmp += *(a->pos()) ;
              i++ ;
            }
          }

          m_tmpBarycenter = tmp / i ;
        }

        // Rotate the selected atoms about the center
        // rotate only selected primitives
        transfAtomRotate_out.matrix().setIdentity();

        // Return to the center of the 3D-space.
        transfAtomRotate_out.translation() = m_tmpBarycenter ;

        // Apply rotations.
        transfAtomRotate_out.rotate(
          Eigen::AngleAxisd( (rotAtomdegX_in/90.0)* 0.1, camBackTransformedYAxis) );

        transfAtomRotate_out.rotate(
          Eigen::AngleAxisd( (rotAtomdegY_in/90.0)*-0.1, camBackTransformedXAxis) );

        /*
        m_transfAtomRotate_out.rotate(
            Eigen::AngleAxisd( m_vectAtomTranslate[1]*WMAVO_ATOM_ROTATION_SPEED,
                        camBackTransformedXAxis)
            );
        m_transfAtomRotate_out.rotate(
            Eigen::AngleAxisd( m_vectAtomTranslate[0]*(-WMAVO_ATOM_ROTATION_SPEED),
                        camBackTransformedYAxis)
            );
        */

        // Return to the object.
        transfAtomRotate_out.translate( -m_tmpBarycenter ) ;
        isMoved = true ;
      }
      else
      { // Put all transformation "at zero".

        transfAtomRotate_out.matrix().setIdentity();
        transfAtomRotate_out.translation() = m_vect3d0 ;

        vectAtomTranslate_out[0] = 0.0 ;
        vectAtomTranslate_out[1] = 0.0 ;
        vectAtomTranslate_out[2] = 0.0 ;

        isMoved = false ;
      }
    }

    return isMoved ;
  }

  /**
    * Receive the "calcul distance" feature from the WmTool class..
    */
  void WrapperChemicalCmdToAvoAction::receiveRequestToCalculDistance()
  {
    m_isCalculDistDiedre = true ;
  }

}
