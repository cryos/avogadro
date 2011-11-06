
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

#include "contextmenu_to_avoaction.h"


namespace Avogadro
{
  ContextMenuToAvoAction::ContextMenuToAvoAction
    ( GLWidget *widget, MoleculeManipulation *mm, WITD::ChemicalWrap *chemWrap )
    
    : m_widget(widget), m_chemWrap(chemWrap), m_moleculeManip(mm),
      m_periodicTable(NULL),
      m_menuActive(false),
      m_contextMenuCurrent(NULL), m_contextMenuMain(NULL),
      m_cancelAct(NULL), m_periodicTableAct(NULL),
      m_contextMenuMeasure(NULL),
      m_noDistAct(NULL), m_distAct(NULL), m_angleAct(NULL), m_diedreAct(NULL),
      m_contextMenuSABF(NULL), m_contextMenuSABFResume(NULL),
      m_insertFragAct(NULL),
      m_addSubstituteFragAct(NULL),
      m_changeAddHAct(NULL), 
      m_contextMenuHydrogen(NULL), m_addAllHAct(NULL), m_removeAllHAct(NULL),
      m_selectAllBondedAtomAct(NULL)
  {

    m_periodicTable = new PeriodicTableView() ;
    buildActionForCM() ;
    buildContextMenu() ;
    
    if( !connectMainQActionSignal() )
    {
      QString msg="Bug: connectMainQActionSignal() method. At least signal is not connected." ;
      mytoolbox::dbgMsg( msg ) ;
    }
  }


  ContextMenuToAvoAction::~ContextMenuToAvoAction()
  {
    if( m_periodicTable != NULL )
      delete( m_periodicTable ) ;
  }


  /**
    * Allocate all QAction object for the context menu.
    */
  void ContextMenuToAvoAction::buildActionForCM()
  {
    m_cancelAct = new QAction( tr("Close menu"), this ) ;
    m_cancelAct->setStatusTip( tr("Close this context menu") ) ;
    //QIcon icon( "/home/mickaelgadroy/Dropbox/Photos/Plan9bunnysmblack.jpg" ) ;
    //m_cancelAct->setIcon( icon ) ;
    //m_cancelAct->setIconVisibleInMenu(true) ;

    m_periodicTableAct = new QAction( tr("Periodic Table"), this ) ;
    m_periodicTableAct->setStatusTip( tr("Display a periodic table") ) ;

    m_noDistAct = new QAction( tr("Clear measure"), this ) ;
    m_noDistAct->setStatusTip( tr("Clear current display of distance, angle & angle diedre") ) ;

    m_distAct = new QAction( tr("Measure distances"), this ) ;
    m_distAct->setStatusTip( tr("Measure & display the distance between atoms") ) ;

    m_angleAct = new QAction( tr("Measure angle between atoms"), this ) ;
    m_angleAct->setStatusTip( tr("Measure & display the angle between atoms") ) ;

    m_diedreAct = new QAction( tr("Measure dihedral angle between atoms"), this ) ;
    m_diedreAct->setStatusTip( tr("Measure & display dihedral angle between atoms") ) ;

    m_changeAddHAct = new QAction( tr("Adjust Hydrogen ..."), this ) ;
    m_changeAddHAct->setCheckable( true ) ;
    #if WMEX_ADJUST_HYDROGEN // Just to initiate the value at begin.
      m_changeAddHAct->setChecked(true) ;
    #else
      m_changeAddHAct->setChecked(false) ;
    #endif
    m_changeAddHAct->setStatusTip( tr("Measure & display dihedral angle between atoms") ) ;

    m_addAllHAct = new QAction( tr("Add all Hydrogen"), this ) ;
    m_addAllHAct->setStatusTip( tr("Add all Hydrogen atoms in the molecule") ) ;

    m_removeAllHAct = new QAction( tr("Remove all Hydrogen"), this ) ;
    m_removeAllHAct->setStatusTip( tr("Remove all Hydrogen atoms in the molecule") ) ;
    
    m_selectAllBondedAtomAct = new QAction( tr("Select all bonded atoms"), this ) ;
    m_selectAllBondedAtomAct = new QAction( tr("Select all bonded atoms of selected atom"), this ) ;
  }

    /**
    * Build the context menu before use it ...
    */
  void ContextMenuToAvoAction::buildContextMenu()
  {
    m_contextMenuMain = new QMenu_ex( m_widget, NULL ) ;
    m_contextMenuMain->setTitle( "WmAvo Context Menu" ) ;
    m_contextMenuMain->setStyleSheet( "QMenu {background-color: #ABABAB;border: 1px solid black;} \
                                        QMenu::item { background-color: transparent; } \
                                        QMenu::item:selected { background-color: #654321; }" ) ;

    
    m_contextMenuMain->addAction( m_periodicTableAct ) ;
    m_contextMenuMain->addMenu( createMenuSubstituteAtomByFragment() ) ;
    m_contextMenuMain->addSeparator() ;

    m_contextMenuMeasure = new QMenu_ex( "Measure", m_contextMenuMain, m_contextMenuMain ) ;
    m_contextMenuMeasure->addAction( m_noDistAct ) ;
    m_contextMenuMeasure->addAction( m_distAct ) ;
    m_contextMenuMeasure->addAction( m_angleAct ) ;
    m_contextMenuMeasure->addAction( m_diedreAct ) ;

    m_contextMenuMain->addMenu( m_contextMenuMeasure ) ;
    m_contextMenuMain->addAction( m_selectAllBondedAtomAct ) ;
    m_contextMenuMain->addSeparator() ;

    m_contextMenuMain->addAction( m_changeAddHAct ) ;

    m_contextMenuHydrogen = new QMenu_ex( "Hydrogen", m_contextMenuMain, m_contextMenuMain ) ;
    m_contextMenuHydrogen->addAction( m_addAllHAct ) ;
    m_contextMenuHydrogen->addAction( m_removeAllHAct ) ;

    m_contextMenuMain->addMenu( m_contextMenuHydrogen ) ;
    m_contextMenuMain->addSeparator() ;

    m_contextMenuMain->addAction( m_cancelAct ) ;

    // Init default actions.
    m_contextMenuMain->setDefaultAction( m_cancelAct ) ;
    m_contextMenuMain->setActiveAction( m_periodicTableAct ) ; // m_contextMenuMain->actions().at(0)
        // Be careful, a 2nd setActiveAction exist below.
  }


  /**
    * Connect the main signals of the context menu. The others are managed dynamically.
    * @return TRUE if all signals are connected ; FALSE else.
    */
  bool ContextMenuToAvoAction::connectMainQActionSignal()
  {
    bool isConnect=false, ok=true ;

    //
    // Connect QAction triggered signals.

    isConnect=connect( m_cancelAct, SIGNAL(triggered()), this, SLOT(closeContextMenu()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_cancelAct.triggered() -> ContextMenuToAvoAction.closeContextMenu() !!" ) ;
      ok = false ;
    }

    isConnect=connect( m_periodicTableAct, SIGNAL(triggered()), m_periodicTable, SLOT(show()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_periodicTableAct.triggered() -> m_periodicTable.show() !!" ) ;
      ok = false ;
    }

    isConnect=connect( m_noDistAct, SIGNAL(triggered()), this, SLOT(askWmToolToCalculNothing()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_noDistAct.triggered() -> ContextMenuToAvoAction.askWmToolToCalculNothing() !!" ) ;
      ok = false ;
    }

    isConnect=connect( m_distAct, SIGNAL(triggered()), this, SLOT(askWmToolToCalculDistance()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_distAct.triggered() -> ContextMenuToAvoAction.askWmToolToCalculDistance() !!" ) ;
      ok = false ;
    }

    isConnect=connect( m_angleAct, SIGNAL(triggered()), this, SLOT(askWmToolToCalculAngle()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_angleAct.triggered() -> ContextMenuToAvoAction.askWmToolToCalculAngle() !!" ) ;
      ok = false ;
    }

    isConnect=connect( m_diedreAct, SIGNAL(triggered()), this, SLOT(askWmToolToCalculDiedre()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_diedreAct.triggered() -> ContextMenuToAvoAction.askWmToolToCalculDiedre() !!" ) ;
      ok = false ;
    }

    isConnect=connect( m_changeAddHAct, SIGNAL(triggered()), m_moleculeManip, SLOT(invertHasAddHydrogen()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_changeAddHAct.triggered() -> m_moleculeManip.invertHasAddHydrogen() !!" ) ;
      ok = false ;
    }

    isConnect=connect( m_addAllHAct, SIGNAL(triggered()), this, SLOT(addAllHydrogens()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_addAllHAct.triggered() -> ContextMenuToAvoAction.addAllHydrogens() !!" ) ;
      ok = false ;
    }

    isConnect=connect( m_removeAllHAct, SIGNAL(triggered()), this, SLOT(removeAllHydrogens()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_removeAllHAct.triggered() -> ContextMenuToAvoAction.removeAllHydrogens() !!" ) ;
      ok = false ;
    }
    
    isConnect=connect( m_selectAllBondedAtomAct, SIGNAL(triggered()), this, SLOT(beginToSelectAllBondedAtom()) ) ;
    if( !isConnect )
    {
      mytoolbox::dbgMsg( "Problem connection signal : m_selectAllBondedAtomAct.triggered() -> ContextMenuToAvoAction.beginToSelectAllBondedAtom() !!" ) ;
      ok = false ;
    }

    return ok ;
  }


  /**
    * Change the size of the context menu.
    */
  void ContextMenuToAvoAction::setFontSizeContextMenu(int ratio)
  {
    float r=(float)(ratio)*0.1f ;
    int fontsize=(int)(r*(float)(WMTOOL_FONT_POINTSIZE_INFO)) ;

    if( r>=WMTOOL_POINTSIZE_RATIO_MIN && r<=WMTOOL_POINTSIZE_RATIO_MAX )
    {
      QFont font( WMTOOL_FONT_FAMILY_INFO, fontsize, WMTOOL_FONT_WEIGHT_INFO ) ;
      m_contextMenuMain->setFont( font ) ;
      m_contextMenuMeasure->setFont( font ) ;
      m_contextMenuSABF->setFont( font ) ;
      m_contextMenuSABFResume->setFont( font ) ;
      m_contextMenuHydrogen->setFont( font ) ;

      for( int i=0 ; i<m_famillyFragAct.size() ; i++ )
        m_famillyFragAct.at(i)->setFont(font) ;
    }
  }


   /**
    * Transform a wrapper action to an Avogadro action : close the periodic table.
    * @param state All actions ask by the wrapper
    * @param posCursor The new position of the mouse cursor
    */
  void ContextMenuToAvoAction::closePeriodicTable( int &state, QPoint posCursor )
  {
    // Close periodic table when visible.
    // CAUTION !!
    // All features using menu must be close before the mouse actions of context menu.
    // Because, the mouse actions of context menu active the opening of feature,
    // so, if we ask isVisible & OK_MENU, it opens a feature with OK_MENU, then this
    // feature takes for it too.
    if( m_periodicTable->isVisible() && WMAVO_IS2(state, WMAVO_MENU_OK) )
    {
      m_periodicTable->setFocus() ;
      //m_periodicTable->grabMouse() ; // Surtout pas !

      // QEvent::MouseButtonPress, QEvent::MouseButtonRelease

      // Select an atomic number (emit a signal).
      QMouseEvent me(QEvent::MouseButtonPress, m_periodicTable->mapFromGlobal(posCursor), Qt::LeftButton, Qt::NoButton, Qt::NoModifier) ;
      QApplication::sendEvent( m_periodicTable->viewport(), &me ) ;

      // Close the periodic table.
      QMouseEvent me2(QEvent::MouseButtonDblClick, m_periodicTable->mapFromGlobal(posCursor), Qt::LeftButton, Qt::NoButton, Qt::NoModifier) ;
      QApplication::sendEvent( m_periodicTable->viewport(), &me2 ) ;

      WITD::ChemicalWrapData_to chemDataTo ;
      chemDataTo.setMenuMode(false) ;
      m_chemWrap->setWrapperDataTo( chemDataTo ) ;
      WMAVO_SETOFF2( state, WMAVO_MENU_ACTIVE ) ;
    }
  }
  
  
  /**
    * Call the method to remove all Hydrogen.
    */
  void ContextMenuToAvoAction::removeAllHydrogens()
  {
    m_moleculeManip->removeHydrogens() ;
    // Update() method is called, because it is not called in wmActions().
    m_widget->update() ;
  }
  
  
  /**
    * Call the method to add all Hydrogen.
    */
  void ContextMenuToAvoAction::addAllHydrogens()
  {
    m_moleculeManip->addHydrogens() ;
    // Update() method is called, because it is not called in wmActions().
    m_widget->update() ;
  }


  /**
    * Transform a wrapper action to an Avogadro action : show the context menu.
    * @param state All actions ask by the wrapper
    * @param posCursor The new position of the mouse cursor
    */
  void ContextMenuToAvoAction::manageAction( int &state, QPoint posCursor )
  {

    if( m_widget->selectedPrimitives().size() >= 2 )
    {
      WITD::ChemicalWrapData_to chemDataTo ;
      chemDataTo.setMenuMode(false) ;
      m_chemWrap->setWrapperDataTo( chemDataTo ) ;
      WMAVO_SETOFF2( state, WMAVO_MENU_ACTIVE ) ;
    }
    else
    {
      // This must be before context menu actions (explain in the method).
      closePeriodicTable( state, posCursor ) ;


      // Menu activation.
      if( WMAVO_IS2(state, WMAVO_MENU_ACTIVE) )
      {
        //QContextMenuEvent cme( QContextMenuEvent::Mouse, QPoint(5,5) ) ;
        //QApplication::sendEvent( m_widget->current(), &cme ) ;

        //cout << "0menu active:" << m_menuActive << endl ;

        if( !m_menuActive && !m_periodicTable->isVisible() )
        {
          //m_periodicTable->show() ;
          //bool hasAddHydrogen=m_moleculeManip->hasAddedHydrogen() ;
          int nbSelectedAtom=0 ;
          bool selectH=false ;

          m_menuActive = true ;
          m_contextMenuCurrent = m_contextMenuMain ;
          m_contextMenuMain->setActiveAction( m_periodicTableAct ) ;

          // Get some information on the selected atoms.
          nbSelectedAtom = m_widget->selectedPrimitives().size() ;
          if( nbSelectedAtom == 1 )
          {
            QList<Primitive*> atomList=m_widget->selectedPrimitives().subList( Primitive::AtomType ) ;
            Atom *a=NULL ;
            if( atomList.size() > 0 )
            {
              #if defined WIN32 || defined _WIN32
              a = dynamic_cast<Atom*>(atomList[0]) ;
              #else
              a = static_cast<Atom*>(atomList[0]) ;
              if( a!=NULL && !a->type()==Primitive::AtomType )
                a = NULL ;
              #endif

              if( a!=NULL && a->isHydrogen() )
                selectH = true ;
            }
          }

          // Initiate the visual of the action.
          m_changeAddHAct->setChecked( m_moleculeManip->hasAddedHydrogen() ) ;
                  
          // Rename/Disable useless menu.
          if( /*hasAddHydrogen &&*/ nbSelectedAtom==1 && selectH )
          { // Substitute Hydrogen By Fragment.
            m_contextMenuSABF->setEnabled(true) ;
            m_contextMenuSABF->setTitle( "Substitute hydrogen by fragment" ) ;
          }
          /*
          else if( !hasAddHydrogen && nbSelectedAtom==1 )
          { // Link atom to Fragment.
            m_contextMenuSABF->setEnabled(true) ;
            m_contextMenuSABF->setTitle( "Bond atom by fragment" ) ;
          }
          */
          else if( nbSelectedAtom == 0 )
          { // Insert Fragment.
            m_contextMenuSABF->setTitle( "Insert fragment" ) ;
            m_contextMenuSABF->setEnabled(true) ;
          }
          else
          { // Do nothing.
            m_contextMenuSABF->setEnabled(false) ;
          }

          // Method 1
          //m_contextMenuMain->show() ; where is setPos !!?

          // Method 2, soit Appel non bloquant.
          m_contextMenuCurrent->popup( posCursor ) ;

          // Method 3, soit appel bloquant
          //m_contextMenuMain->exec( posCursor ) ;
          // => return QAction* : either realized QAction, either NULL=echap

          // Here, do not try to realize action according to the return
          // because it realizes by predefined signals.
        }
        else
        {
          // If the context menu disappears.
          if( !m_contextMenuCurrent->isVisible() && !m_periodicTable->isVisible() )
          {
            /*
            if( m_contextMenuCurrent->getMenuParent() == NULL )
              // The sub-menu can reappear on the menu ...
              m_contextMenuCurrent->popup( QPoint(posCursor.x()-30, posCursor.y()-30) ) ;
            else
            */
              m_contextMenuCurrent->popup( posCursor ) ;
          }
        }

        //cout << "  menu active:" << m_menuActive << endl ;


        if( WMAVO_IS2(state,WMAVO_MENU_OK) )
        {
          //cout << "  menu OK" << endl ;
          //QAction *actionMenu=m_contextMenuMain->actionAt( m_contextMenuMain->mapFromGlobal(posCursor) ) ;
          QAction *actionMenu=m_contextMenuCurrent->activeAction() ;

          if( actionMenu != NULL )
          {
            // The test is here to limit the time between the work realized by
            // the action called by trigger(), and limit the risk that wmavo returns
            // a signal to execut another instance of this function with bad parameters ...
             

            #if defined WIN32 || defined _WIN32
            QMenu_ex *cm=dynamic_cast<QMenu_ex*>(m_contextMenuCurrent->activeAction()->menu()) ;
            #else
            QMenu_ex *cm=static_cast<QMenu_ex*>(m_contextMenuCurrent->activeAction()->menu()) ;
            #endif

            // Test if the current action is a "menu" (a root to go to another menu).
            if( cm==NULL || cm==m_contextMenuCurrent )
            {
              if( actionMenu->text().compare(tr("Periodic Table")) != 0 )
              {
                WITD::ChemicalWrapData_to chemDataTo ;
                chemDataTo.setMenuMode(false) ;
                m_chemWrap->setWrapperDataTo( chemDataTo ) ;
                WMAVO_SETOFF2( state, WMAVO_MENU_ACTIVE ) ;
              }

              // Trigger the associated action.
              actionMenu->trigger() ;

              // Then close the menu.
              m_menuActive = false ;
              m_contextMenuMain->close() ;
            }
            else
            {
              if( cm!=NULL && cm->isEnabled() )
              {
                QKeyEvent ke(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier) ;
                QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;
                if( cm!=NULL && cm!=m_contextMenuCurrent )
                  m_contextMenuCurrent = cm ;
              }
            }
          }
          else
          { // No active menu => Close the context menu.

            m_menuActive = false ;
            m_contextMenuMain->close() ;
            WITD::ChemicalWrapData_to chemDataTo ;
            chemDataTo.setMenuMode(false) ;
            m_chemWrap->setWrapperDataTo( chemDataTo ) ;
            WMAVO_SETOFF2( state, WMAVO_MENU_ACTIVE ) ;
          }
        }

        //QWidget *widgetTmp=m_contextMenuMain->focusWidget() ;

        //if( widgetTmp != NULL )
        //{
          else if( WMAVO_IS2(state, WMAVO_MENU_DOWN) )
          {
            QKeyEvent ke(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier) ;
            QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;
          }

          else if( WMAVO_IS2(state, WMAVO_MENU_UP) )
          {
            QKeyEvent ke(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier) ;
            QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;
          }

          else if( WMAVO_IS2(state, WMAVO_MENU_RIGHT) )
          {
            QAction *qa=m_contextMenuCurrent->activeAction() ;
            QMenu_ex *cm1=( qa==NULL ? NULL : dynamic_cast<QMenu_ex*>(qa->menu()) ) ;
            //if( qa!=NULL && cm1!=NULL )
              //printf( "%s:%s:%s\n", cm1->title().toStdString().c_str(), qa->text().toStdString().c_str(), qa->objectName().toStdString().c_str() ) ;

            if( cm1!=NULL && (cm1->title()==qa->text() && cm1->isEnabled()) )
            {
              QKeyEvent ke(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier) ;
              QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;

              if( m_contextMenuCurrent->activeAction() != NULL )
              {
                #if defined WIN32 || defined _WIN32
                QMenu_ex *cm=dynamic_cast<QMenu_ex*>(m_contextMenuCurrent->activeAction()->menu()) ;
                #else
                QMenu_ex *cm=static_cast<QMenu_ex*>(m_contextMenuCurrent->activeAction()->menu()) ;
                #endif
                
                if( cm!=NULL && cm!=m_contextMenuCurrent )
                  m_contextMenuCurrent = cm ;
              }
            }
          }

          else if( WMAVO_IS2(state, WMAVO_MENU_LEFT) )
          {
            QKeyEvent ke(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier) ;
            QApplication::sendEvent( m_contextMenuCurrent, &ke ) ;

            if( m_contextMenuCurrent->getMenuParent() != NULL )
            {
              m_contextMenuCurrent->close() ;
              m_contextMenuCurrent = m_contextMenuCurrent->getMenuParent() ;
            }
          }
        //}
      }
      else
      {
        // Desactivate all context menu & Co.
        m_contextMenuMain->close() ;
      }
    }
  }


  /**
    * Create the "Substitute Atom By Fragment" sub-menu for the context menu.
    * It lets to search in an Avogadro repository a lot of fragment files.
    * @return A (QMenu_ex*) object which represents the "Substitute Atom By Fragment" sub-menu.
    */
  QMenu_ex* ContextMenuToAvoAction::createMenuSubstituteAtomByFragment()
  {
    QDir fragRootDir( QCoreApplication::applicationDirPath()+"/../share/avogadro/fragments/" ) ;

    //mytoolbox::dbgMsg( fragRootDir.absolutePath() ;
    //QStringList filtersName ;
    //filtersName << "*.cpp" << "*.cxx" << "*.cc" ;
    //fragRootDir.setNameFilters( filtersName ) ;

    m_contextMenuSABF = createMenuSABF(m_contextMenuMain, fragRootDir) ;

    if( m_contextMenuSABF != NULL )
    {
      m_contextMenuSABF->insertMenu( m_contextMenuSABF->actions().at(0),
                                     createMenuResumeSubstituteAtomByFragment(m_contextMenuSABF) ) ;
      m_contextMenuSABF->setTitle( "Fragment" ) ;
    }

    return m_contextMenuSABF ;
  }


  /**
    * Create the "Substitute Atom By Fragment" resume sub-menu for the context menu.
    * It lets to search in an Avogadro repository some selected fragment files.
    * @return A (QMenu_ex*) object which represents the "Substitute Atom By Fragment" sub-menu.
    */
  QMenu_ex* ContextMenuToAvoAction::createMenuResumeSubstituteAtomByFragment( QMenu_ex *parent )
  {
    /* 1st method, if the cmake can copy img/ directory in an Avogadro directory.
    QDir fragRootDir( QCoreApplication::applicationDirPath()+"/../share/avogadro/fragments_resume" ) ;

    if( fragRootDir.exists() )
    {
      m_contextMenuSABF = createMenuSABF(m_contextMenuMain, fragRootDir, false ) ;
      m_contextMenuSABF->setTitle( "Substitute atom by fragment (resume)" ) ;
    }
    */

    QDir fragRootDir( QCoreApplication::applicationDirPath()+"/../share/avogadro/fragments" ) ;

    if( fragRootDir.exists() )
    {  
      QString absPath=fragRootDir.toNativeSeparators( fragRootDir.absolutePath() ) + fragRootDir.separator() ;

      QVector<QString> listFragment ;
      QString tmp ;
      QAction_ex *aTmp=NULL ;
      bool isConnect ;

      listFragment
        << "methyl.cml" << "-CH3"
        << "alkenes/ethene.cml" << "-CH=CH2"
        << "aldehydes/formaldehyde.cml" << "-HC=O"
        << "carboxylic_acids/formic_acid.cml" << "-COOH"
        << "amines/ammonia.cml" << "-NH2"
        << "alcohols/alcohol.cml" << "-OH"
        << "alkynes/acetylene.cml" << "-actylene"
        << "cyclic alkanes/cyclopentane.cml" << "-cyclopentane"
        << "cyclic alkanes/cyclohexane.cml" << "-cyclohexane"
        << "aromatics/benzene.cml" << "-phenyl"
        << "heteroaromatics/1H-pyrrole.cml" << "-pyrrole"
        << "chlorine.cml" << "-Cl"
        << "bromine.cml" << "-Br"
        << "iodine.cml" << "-I"
        << "metoxy.cml" << "-ether"
        << "thiols/thiol.cml" << "-SH"
        << "nitrogen dioxyde.cml" << "-NO2" ;

      m_contextMenuSABFResume = new QMenu_ex( "Frequently used", parent, parent ) ;
     
      // Get files.
      for( int i=0 ; i<listFragment.size() ; i+=2 )
      {
        tmp = absPath ;
        tmp += listFragment[i] ;

        aTmp = new QAction_ex( listFragment[i+1], this ) ;
        m_fragAct.append( aTmp ) ;
        aTmp->setStatusTip( tmp ) ;

        isConnect = connect( aTmp, SIGNAL(triggeredInfo(QString)),
                             this, SLOT(letToSubstituteAtomByFragment(QString)) ) ;

        if( !isConnect )
          mytoolbox::dbgMsg( "Problem connection signal : ActionsModified.triggeredInfo() -> ContextMenuToAvoAction.letToSubstituteAtomByFragment() !!" ) ;

        m_contextMenuSABFResume->addAction( m_fragAct.last() ) ;
      }
    }

    return m_contextMenuSABFResume ;
  }


  /**
    * Help to create the "Substitute Atom By Fragment" sub-menu for the context menu.
    * This is a recursive method to search in an Avogadro repertory.
    * It is called by ContextMenuToAvoAction::createMenuSubstituteAtomByFragment() .
    * @return A (QMenu_ex*) object which represents a big part of "Substitute Atom By Fragment" sub-menu.
    * @param parent The (QMenu_ex*) parent object.
    * @param dirCur The directory which contains all fragments
    * @param withFamily If a family tree is created or not.
    */
  QMenu_ex* ContextMenuToAvoAction::createMenuSABF( QMenu_ex *parent, QDir dirCur, bool withFamily )
  {
    bool isConnect=false ;
    //QStringList filtersName ;
    //filtersName << "*.cml" ;
    //dirCur.setNameFilters( filtersName ) ;

    QAction_ex *aTmp=NULL ;
    QString tmpStr ;
    QMenu_ex *cmCur=parent ; // Current QMenu.
    QDir::Filters filterDir( QDir::Dirs | QDir::NoDotAndDotDot ) ;
    QDir::Filters filterFile( QDir::Files ) ;
    QStringList dirs=dirCur.entryList( filterDir, QDir::Name ) ; // Get directories entries.
    QStringList files=dirCur.entryList( filterFile, QDir::Name ) ; // Get files entries.
    QString absPath=dirCur.toNativeSeparators( dirCur.absolutePath() ) + dirCur.separator() ;

    // Create & append current menu.
    if( withFamily )
    {
      m_famillyFragAct.append( new QMenu_ex(dirCur.dirName(), parent, parent) ) ;
      cmCur = m_famillyFragAct.last() ;
    }

    // Get directories.
    foreach( QString d, dirs )
    {
      tmpStr = absPath ;
      tmpStr += d ;
      //mytoolbox::dbgMsg( tmpStr ) ;

      if( withFamily )
        cmCur->addMenu( createMenuSABF( cmCur, QDir(tmpStr)) ) ;
      else
        createMenuSABF( cmCur, QDir(tmpStr)) ;
    }

    // Get files.
    foreach( QString f, files )
    {
      tmpStr = absPath ;
      tmpStr += f ;
      //mytoolbox::dbgMsg( tmpStr ) ;

      aTmp = new QAction_ex(f, this) ;
      m_fragAct.append( aTmp ) ;
      aTmp->setStatusTip( tmpStr ) ;

      isConnect = connect( aTmp, SIGNAL(triggeredInfo(QString)),
                           this, SLOT(letToSubstituteAtomByFragment(QString)) ) ;

      if( !isConnect )
        mytoolbox::dbgMsg( "Problem connection signal : ActionsModified.triggeredInfo() -> ContextMenuToAvoAction.letToSubstituteAtomByFragment() !!" ) ;

      cmCur->addAction( m_fragAct.last() ) ;
    }

    return cmCur ;
  }


  /**
    * Slot uses to close the context menu by the context menu itself for example.
    */
  void ContextMenuToAvoAction::closeContextMenu()
  {
    if( m_contextMenuMain != NULL )
    {
      //m_contextMenuMain->hide() :
      m_contextMenuMain->close() ;
      m_menuActive = false ;
    }
    else
      mytoolbox::dbgMsg( "Bug in ContextMenuToAvoAction::closeContextMenum() : A NULL-OBJECT not expected." ) ;
  }

  void ContextMenuToAvoAction::letToSubstituteAtomByFragment( QString fragmentAbsPath )
  {
    QList<Primitive*> selectedAtoms=m_widget->selectedPrimitives().subList( Primitive::AtomType ) ;
    QList<QString> strList ;
    Atom *a=NULL ;

    if( selectedAtoms.size() == 1 )
    {
      #if defined WIN32 || defined _WIN32
      a = dynamic_cast<Atom*>(selectedAtoms[0]) ;
      #else
      a = static_cast<Atom*>(selectedAtoms[0]) ;
      if( a!=NULL && !a->type()==Primitive::AtomType )
        a = NULL ;
      #endif
    }

    if( selectedAtoms.size() > 1 )
    {
      strList << "Erreur lors de l'ajout d'un fragment :" 
              << "Un SEUL atome ou aucun atome doit etre selectionne." ;
      emit displayedMsg( strList, QPoint(300,20) ) ;
      mytoolbox::dbgMsg( strList ) ;
    }
    /*
    else if( m_changeAddHAct->isChecked() && !a->isHydrogen() )
    {
      strList << "Erreur lors de l'ajout d'un fragment :" 
              << "Substitution possible seulement pour un atome d'Hydrogene." ;
      emit displayedMsg( strList, QPoint(300,20) ) ;
      mytoolbox::dbgMsg( strList ) ;
    }
    */
    else
    { // All is OK

      PrimitiveList* newElement=NULL ;
      newElement = m_moleculeManip->addFragment( fragmentAbsPath, a ) ;

      if( newElement != NULL )
      {
        // Update() method is called, because it is not called in wmActions().
        m_widget->update() ;

        m_widget->clearSelected() ;
        m_widget->setSelected( *newElement, true ) ;
        delete newElement ;
      }
    }
  }


  /**
    * Let the ContextMenuToAvoAction class to ask the WmTool class to calcul and display nothing ...
    * In fact, this method informs the WmTool class to cancel the current actions using the calcul
    * of distance, angle and diedre angle between atoms.
    */
  void ContextMenuToAvoAction::askWmToolToCalculNothing()
  {
    //cout << "nothing" << endl ;
    emit initiatedCalculDistDiedre( -1 ) ;
  }


  /**
    * Let the ContextMenuToAvoAction class to ask the WmTool to calcul and display the distance between 2 atoms.
    */
  void ContextMenuToAvoAction::askWmToolToCalculDistance()
  {
    //cout << "distance" << endl ;
    emit initiatedCalculDistDiedre( 2 ) ;
  }


  /**
    * Let the ContextMenuToAvoAction class to ask the WmTool to calcul and display the angle between 3 atoms.
    */
  void ContextMenuToAvoAction::askWmToolToCalculAngle()
  {
    //cout << "angle" << endl ;
    emit initiatedCalculDistDiedre( 3 ) ;
  }


  /**
    * Let the ContextMenuToAvoAction class to ask the WmTool to calcul and display the diedre angle between 4 atoms.
    */
  void ContextMenuToAvoAction::askWmToolToCalculDiedre()
  {
    //cout << "diedre" << endl ;
    emit initiatedCalculDistDiedre( 4 ) ;
  }
  
  
  /**
   * Launch the method to select all bonded atoms with the selected atoms.
   */
  void ContextMenuToAvoAction::beginToSelectAllBondedAtom()
  {
    PrimitiveList selectedPrim1=m_widget->selectedPrimitives() ;
    PrimitiveList *selectedPrim2=m_moleculeManip->getAllBondedAtom( selectedPrim1 ) ;
    
    
    if( selectedPrim2 != NULL )
    {
      m_widget->setSelected( *selectedPrim2, true ) ;
      delete selectedPrim2 ;
      
      // Update() method is called, because it is not called in wmActions().
      m_widget->update() ;
    }
  }

}
