/**********************************************************************
  GAMESS - GAMESS Input Deck Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "gamessextension.h"

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <openbabel/obconversion.h>

#include <QDockWidget>
#include <QPushButton>
#include <QTreeView>
#include <QVBoxLayout>

#include <QtGui>
#include <QFrame>
#include <QSpacerItem>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  enum GamessExtensionIndex {
    InputDeckAction = 0,
    EFPAction,
    QMAction
  };

  GamessExtension::GamessExtension( QObject *parent ) : QObject( parent ), m_inputDialog( NULL ), m_inputData( NULL ), m_dockWidget( 0 ),
                                    m_efpDialog(0), m_qmDialog(0)
  {
    Action *action = new Action( this );
    action->setText( "Input Deck Generator" );
    action->setMenuPath( "&Tools>GAMESS" );
    m_actions.append( action );
    action->setData( InputDeckAction );

    action = new Action( this );
    action->setText( "EFP Selection" );
    action->setMenuPath( "&Tools>GAMESS" );
    m_actions.append( action );
    action->setData( EFPAction );

    action = new Action( this );
    action->setText( "QM Selection" );
    action->setMenuPath( "&Tools>GAMESS" );
    m_actions.append( action );
    action->setData( QMAction );
  }

  GamessExtension::~GamessExtension()
  {
    if(m_efpDialog)
    {
      m_efpDialog->close();
    }

    if(m_qmDialog)
    {
      m_qmDialog->close();
    }

  }

  QList<Action *> GamessExtension::actions() const
  {
    return m_actions;
  }

  QDockWidget * GamessExtension::dockWidget()
  {
    if ( !m_dockWidget ) {
      m_dockWidget = new QDockWidget( tr( "GAMESS EFP Information" ) );

      QWidget *widget = new QWidget( m_dockWidget );
      QVBoxLayout *layout = new QVBoxLayout();

      QTreeView *view= new QTreeView();
      layout->addWidget( view );

      widget->setLayout( layout );
      m_dockWidget->setWidget( widget );
      m_dockWidget->setVisible( false );

      connect( m_dockWidget, SIGNAL( destroyed() ), this, SLOT( dockWidgetDestroyed ) );
    }

    return m_dockWidget;
  }

  void GamessExtension::dockWidgetDestroyed()
  {
    m_dockWidget = 0;
  }

  QUndoCommand* GamessExtension::performAction( Action *action, Molecule *molecule, GLWidget *widget, QTextEdit * )
  {

    qDebug() << "Perform Action";
    int i = action->data().toInt();
    switch ( i ) {
      case InputDeckAction:
        if ( !m_inputData ) {
          m_inputData = new GamessInputData( molecule );
        } else {
          m_inputData->SetMolecule( molecule );
        }
        if ( !m_inputDialog ) {
          m_inputDialog = new GamessInputDialog( m_inputData );
          m_inputDialog->show();
        } else {
          m_inputDialog->setInputData( m_inputData );
          m_inputDialog->show();
        }
        break;
      case EFPAction:
        if(!m_efpDialog)
        {
          m_efpDialog = matchesDialog( molecule, widget, GamessEfpMatchDialog::EFPType );
        }
        else
        {
          m_efpDialog->show();
          m_efpDialog->raise();
        }
        break;
      case QMAction:
        if(!m_qmDialog)
        {
          m_qmDialog = matchesDialog( molecule, widget, GamessEfpMatchDialog::QMType );
        }
        else
        {
          m_qmDialog->show();
          m_qmDialog->raise();
        }
        break;
    }

    return 0;
  }

  GamessEfpMatchDialog *GamessExtension::matchesDialog( Molecule *molecule, GLWidget *widget, GamessEfpMatchDialog::Type type )
  {
    qDebug() << "Find Matches";

    if ( !widget->selectedPrimitives().size() ) {
      QMessageBox::information( 0, tr( "No Atoms Selected" ),
                                tr( "You must make a selection!" ) );
      return 0;
    }

    OBConversion conv;

    if ( !conv.SetOutFormat( "smi" ) ) {
      QMessageBox::critical( 0, tr( "SMILES Conversion Unavailable" ),
                             tr( "SMILES Format Conversion Not Available!" ) );
      return 0;
    }

    QList<Primitive *> selectedPrimitives = widget->selectedPrimitives();
    Molecule selectedMolecule;

    int numAtoms = 0;
    std::map<OBAtom*, OBAtom*> AtomMap; // key is from old, value from new
    foreach( Primitive *p, selectedPrimitives ) {
      if ( p->type() == Primitive::AtomType ) {
        OBAtom *selected = static_cast<Atom*>( p );
        selectedMolecule.InsertAtom( *selected );
        AtomMap[selected] = selectedMolecule.GetAtom( selectedMolecule.NumAtoms() );
        numAtoms++;
      }
    }

    // use the atom map to map bonds
    int numBonds = 0;
    map<OBAtom*, OBAtom*>::iterator posBegin, posEnd;
    FOR_BONDS_OF_MOL( b, molecule ) {
      posBegin = AtomMap.find( b->GetBeginAtom() );
      posEnd = AtomMap.find( b->GetEndAtom() );
      // make sure both bonds are in the map (i.e. selected)
      if ( posBegin != AtomMap.end() && posEnd != AtomMap.end() ) {
        selectedMolecule.AddBond(( posBegin->second )->GetIdx(),
                                 ( posEnd->second )->GetIdx(),
                                 b->GetBO(), b->GetFlags() );
        numBonds++;
      }
    } // end looping over bonds

    string pattern = conv.WriteString( &selectedMolecule );
    pattern.erase( pattern.find_first_of( " \t\n\r" ) );

//     QMessageBox::information( 0, tr( "" ), "Got Pattern : " + QString::fromStdString( pattern ) );

    OBSmartsPattern sp;
    sp.Init( pattern );

    if ( sp.Match( *molecule ) ) {
      QStandardItemModel *model = new QStandardItemModel();

      vector< vector<int> > maplist = sp.GetUMapList();

      for ( vector< vector<int> >::iterator it1 = maplist.begin(); it1 != maplist.end(); it1++ )
      {

        QVector<int> matches = QVector<int>::fromStdVector(*it1);

        QString text;
        bool valid = true;
        bool first = true;
        foreach(int i, matches)
        {
          OBAtom *atom = molecule->GetAtom(i);

          if(!first) {
            text.append(tr(", "));
          }
          else
          {
            first = false;
          }

          text.append(QString::number(i));

          FOR_NBORS_OF_ATOM( a, atom )
          {
            // all connected atoms must also be in our match
            // see if each neighbor is a hydrogen or another match
            if(a->IsHydrogen())
            {
              if(!first) {
                text.append(tr(", "));
              }
              int idx = a->GetIdx();
              text.append(QString::number(idx));
              matches.append(idx);
            }
            else if( !matches.contains( a->GetIdx()) )
            {
              valid = false;
              break;
            }
          }

          if(!valid) {
            break;
          }


        }

        if(valid)
        {
          QStandardItem *item = new QStandardItem();

          item->setText(text);
          item->setEditable(false);
          item->setData(qVariantFromValue(matches));

          model->appendRow(item);
        }

      }

      GamessEfpMatchDialog *efpDialog = new GamessEfpMatchDialog(model, molecule, widget, type);

      efpDialog->show();

      return efpDialog;
    }

    return 0;
  }

}

#include "gamessextension.moc"

Q_EXPORT_PLUGIN2( gamessextension, Avogadro::GamessExtensionFactory )
