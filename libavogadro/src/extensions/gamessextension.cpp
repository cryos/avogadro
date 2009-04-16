/**********************************************************************
  GAMESS - GAMESS Input Deck Plugin for Avogadro

  Copyright (C) 2006 by Donald Ephraim Curtis
  Copyright (C) 2006 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/mol.h>
#include <openbabel/obiter.h>
#include <openbabel/obconversion.h>

#include <QDockWidget>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

#include <QFrame>
#include <QSpacerItem>
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>

using namespace std;
using namespace OpenBabel;

namespace Avogadro
{

  enum GamessExtensionIndex {
    InputDeckAction = 0,
    EFPAction,
    QMAction
  };

  GamessExtension::GamessExtension( QObject *parent ) : Extension( parent ), m_inputDialog( NULL ), m_inputData( new GamessInputData() ), m_gamessEfpDock( 0 ),
      m_efpModel( new QStandardItemModel() ), m_molecule(0),
      m_efpDialog( 0 ), m_qmDialog( 0 )
  {
    QAction *action = new QAction( this );
    action->setText( tr("Input Generator..." ));
    m_actions.append( action );
    action->setData( InputDeckAction );

    action = new QAction( this );
    action->setText( tr("EFP Selection..." ));
    m_actions.append( action );
    action->setData( EFPAction );

    action = new QAction( this );
    action->setText( tr("QM Selection..." ));
    m_actions.append( action );
    action->setData( QMAction );

  }

  GamessExtension::~GamessExtension()
  {
    if(m_inputDialog)
    {
      m_inputDialog->close();
      m_inputDialog->deleteLater();
    }
  }

  QList<QAction *> GamessExtension::actions() const
  {
    return m_actions;
  }

  QString GamessExtension::menuPath(QAction *) const
  {
    return tr("E&xtensions") + '>' + tr("&GAMESS");
  }

  QDockWidget * GamessExtension::dockWidget()
  {
    if ( !m_gamessEfpDock ) {
      m_gamessEfpDock = new QDockWidget( tr( "GAMESS EFP Information" ) );
      m_gamessEfpDock->setObjectName(tr("gamessEfpDock"));

      QWidget *widget = new QWidget( m_gamessEfpDock );
      QVBoxLayout *layout = new QVBoxLayout();

      m_efpView = new QTreeView();
      m_efpView->header()->setVisible( false );
      layout->addWidget( m_efpView );
      m_efpView->setModel( m_efpModel );
      m_efpView->setSelectionMode(QAbstractItemView::ExtendedSelection);
      connect(m_efpView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
              this, SLOT(efpViewSelectionChanged(QItemSelection, QItemSelection)));

      m_efpButton = new QPushButton();
      m_efpButton->setText(tr("Delete"));
      m_efpButton->setDisabled(true);
      layout->addWidget(m_efpButton);
      connect(m_efpButton, SIGNAL(pressed()), this, SLOT(efpButtonPressed()));


      widget->setLayout( layout );
      m_gamessEfpDock->setWidget( widget );
      m_gamessEfpDock->setVisible( false );

      connect( m_gamessEfpDock, SIGNAL( destroyed() ), this, SLOT( dockWidgetDestroyed() ) );
    }

    return m_gamessEfpDock;
  }

  void GamessExtension::dockWidgetDestroyed()
  {
    m_gamessEfpDock = 0;
    m_efpView = 0;
  }

  void GamessExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;

    // clear the EFP data
    m_inputData->EFP->RemoveGroups();
    m_inputData->SetMolecule( molecule );

    connect(molecule, SIGNAL(primitiveRemoved(Primitive *)),
            this, SLOT(removePrimitive(Primitive *)));
  }


  QUndoCommand* GamessExtension::performAction( QAction *action, GLWidget *widget)
  {

    int i = action->data().toInt();

    switch ( i ) {
      case InputDeckAction:
        if ( !m_inputDialog ) {
          m_inputDialog = new GamessInputDialog( m_inputData );
          m_inputDialog->show();
        } else {
          m_inputDialog->setInputData( m_inputData );
          m_inputDialog->show();
        }
        break;
      case EFPAction:
        if ( !m_efpDialog ) {
          m_efpDialog = matchesDialog( m_molecule, widget, GamessEfpMatchDialog::EFPType );
          m_widgetSelected.insert( widget, widget->selectedPrimitives() );
          m_dialogWidgets.insert( m_efpDialog, widget );
          m_dialogMolecules.insert( m_efpDialog, m_molecule );
        } else {
          m_efpDialog->show();
          m_efpDialog->raise();
        }
        break;
      case QMAction:
        if ( !m_qmDialog ) {
          m_qmDialog = matchesDialog( m_molecule, widget, GamessEfpMatchDialog::QMType );
          m_widgetSelected.insert( widget, widget->selectedPrimitives() );
          m_dialogWidgets.insert( m_qmDialog, widget );
          m_dialogMolecules.insert( m_qmDialog, m_molecule );
        } else {
          m_qmDialog->show();
          m_qmDialog->raise();

        }
        break;
    }

    return 0;
  }

  GamessEfpMatchDialog *GamessExtension::matchesDialog(Molecule *molecule,
       GLWidget *widget, GamessEfpMatchDialog::Type type)
  {
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

    m_molecule = molecule;

    PrimitiveList selectedPrimitives = widget->selectedPrimitives();
    Molecule selectedMolecule;

    foreach(Primitive *p, selectedPrimitives.subList(Primitive::AtomType)) {
      Atom *selected = static_cast<Atom*>(p);
      Atom *atom = selectedMolecule.addAtom(selected->id());
      *atom = *selected;
    }

    // use the atom map to map bonds
    foreach (Bond *b, m_molecule->bonds()) {
      // make sure both bonds are in the map (i.e. selected)
      if (selectedMolecule.atomById(b->beginAtomId()) &&
          selectedMolecule.atomById(b->endAtomId())) {
        Bond *bond = selectedMolecule.addBond(b->id());
        bond->setAtoms(b->beginAtomId(), b->endAtomId(), b->order());
      }
    } // end looping over bonds

    // get the SMARTS pattern
    OpenBabel::OBMol obmol = selectedMolecule.OBMol();
    string pattern = conv.WriteString( &obmol );
    pattern.erase( pattern.find_first_of( " \t\n\r" ) );

//    QMessageBox::information( 0, tr( "" ), "Got Pattern : " + QString::fromStdString( pattern ) );

    OBSmartsPattern sp;
    sp.Init( pattern );

    if (sp.Match(obmol)) {
      // before we begin we need to see what has already been selected.
      QVector<Atom *> usedAtoms;
      for(int parentNum = 0; parentNum < m_efpModel->rowCount(); parentNum++ ) {
        QStandardItem *parentItem = m_efpModel->item(parentNum);

        for(int childNum = 0; childNum < parentItem->rowCount(); childNum++ ) {
          QStandardItem *childItem = parentItem->child(childNum);
          QVector<Atom *> atoms = childItem->data().value<QVector<Atom *> >();

          usedAtoms += atoms;
        }
      }

      QStandardItemModel *model = new QStandardItemModel();
      QModelIndex selectedIndex;

      vector< vector<int> > maplist = sp.GetUMapList();

      for (vector< vector<int> >::iterator it1 = maplist.begin();
           it1 != maplist.end(); it1++) {

        QVector<int> matches = QVector<int>::fromStdVector( *it1 );

        QString text;
        bool valid = true;
        bool first = true;
        bool selected = false;
        QVector<Atom *> atomMatches;
        foreach(int i, matches) {
          Atom *atom = selectedMolecule.atom(i-1);

          if(usedAtoms.contains(m_molecule->atomById(atom->id()))) {
            valid=false;
            break;
          }

          atomMatches.append(m_molecule->atomById(atom->id()));
          // if this matches our original atom
          if(!selected && selectedPrimitives.contains(m_molecule->atomById(atom->id()))) {
            selected = true;
          }

          if ( !first ) {
            text.append( tr( ", " ) );
          }
          else {
            first = false;
          }

          text.append( QString::number( i ) );

          foreach (unsigned long ai, atom->neighbors()) {
            Atom *a = m_molecule->atom(ai);
            // all connected atoms must also be in our match
            // see if each neighbor is a hydrogen or another match
            if ( a->isHydrogen() ) {
              if ( !first ) {
                text.append( tr( ", " ) );
              }
              int idx = m_molecule->atomById(a->id())->index();
              text.append(QString::number(idx));
              atomMatches.append((m_molecule->atomById(a->id())));
            }
            else if (!matches.contains(selectedMolecule.atomById(a->id())->index()+1)) {
              valid = false;
              break;
            }
          }

          if ( !valid ) {
            break;
          }
        }

        if ( valid ) {
          QStandardItem *item = new QStandardItem();

          item->setText( text );
          item->setEditable( false );
          item->setData( qVariantFromValue( atomMatches ) );

          model->appendRow( item );

          if(!selectedIndex.isValid() && selected)
          {
            selectedIndex = model->indexFromItem(item);
          }
        }

      }

      GamessEfpMatchDialog *efpDialog = new GamessEfpMatchDialog( model, type, widget );

      efpDialog->select( selectedIndex );
      efpDialog->setAttribute( Qt::WA_DeleteOnClose );

      connect( efpDialog, SIGNAL( accepted( GamessEfpMatchDialog::Type, QString, QList<QVector<Atom *> > ) ),
               this, SLOT( efpWidgetAccepted( GamessEfpMatchDialog::Type, QString, QList<QVector<Atom *> > ) ) );
      connect( efpDialog, SIGNAL( finished(int) ), this, SLOT(efpWidgetDone() ) );
      connect( efpDialog, SIGNAL( selectionChanged( QList<QVector<Atom *> > ) ),
               this, SLOT( efpWidgetSelected( QList<QVector<Atom *> > ) ) );
      efpDialog->show();

      return efpDialog;
    }

    return 0;
  }

  void GamessExtension::efpButtonPressed()
  {
    QModelIndexList selectedIndexes = m_efpView->selectionModel()->selectedRows();

    qSort(selectedIndexes.begin(), selectedIndexes.end(), qGreater<QModelIndex>());
    foreach(QModelIndex index, selectedIndexes)
    {
      QModelIndex parent = index.parent();
      if(!parent.isValid())
      {
        for(int i = 0; i<m_efpModel->rowCount(index); i++)
        {
          QModelIndex child = m_efpModel->index(i,0,index);

          QVector<Atom *> atoms = child.data(Qt::UserRole +1).value<QVector<Atom *> >();
          Atom *atom = atoms.first();

          m_inputData->EFP->RemoveGroups(atom);
        }

        m_efpModel->removeRow(index.row(), parent);
      }
    }

    selectedIndexes = m_efpView->selectionModel()->selectedRows();
    qSort(selectedIndexes.begin(), selectedIndexes.end(), qGreater<QModelIndex>());
    foreach(QModelIndex index, selectedIndexes)
    {
      QModelIndex parent = index.parent();
      if(parent.isValid())
      {
        QVector<Atom *> atoms = index.data(Qt::UserRole +1).value<QVector<Atom *> >();
        Atom *atom = atoms.first();

        m_inputData->EFP->RemoveGroups(atom);

        m_efpModel->removeRow(index.row(), parent);
        if(m_efpModel->rowCount(parent) == 0)
        {
          m_efpModel->removeRow(parent.row());
        }
      }
    }

    if(m_inputDialog)
    {
      m_inputDialog->updatePreviewText();
    }
  }

  void GamessExtension::efpViewSelectionChanged( const QItemSelection &, const QItemSelection & )
  {
    QModelIndexList selectedIndexes = m_efpView->selectionModel()->selectedRows();

    QList<Primitive *> primitives;
    bool enable = false;
    foreach(QModelIndex index, selectedIndexes)
    {
      enable = true;
      if(index.parent().isValid())
      {
        QVector<Atom *> atoms = index.data(Qt::UserRole +1).value<QVector<Atom *> >();
        foreach(Atom *atom, atoms)
        {
          primitives.append(atom);
        }
      }
      else
      {
        int childCount = m_efpModel->rowCount(index);
        for(int i = 0; i<childCount; i++)
        {
          QModelIndex child = m_efpModel->index(i,0,index);
          QVector<Atom *> atoms = child.data(Qt::UserRole +1).value<QVector<Atom *> >();
          foreach(Atom *atom, atoms)
          {
            primitives.append(atom);
          }
        }
      }
    }
    GLWidget *widget = GLWidget::current();

    if(widget)
    {
      widget->clearSelected();
      widget->setSelected( primitives, true );
      widget->update();
    }

    m_efpButton->setEnabled(enable);
  }

  void GamessExtension::efpWidgetSelected( const QList<QVector<Atom *> > &groups )
  {
    GamessEfpMatchDialog *dialog = qobject_cast<GamessEfpMatchDialog *>( sender() );

    GLWidget *widget = m_dialogWidgets.value( dialog );

    QList<Primitive *> primitives;
    foreach( QVector<Atom *> group, groups ) {
      foreach( Atom *atom, group ) {
        primitives.append( atom );
      }
    }

    widget->clearSelected();
    widget->setSelected( primitives, true );
    widget->update();

  }

  void GamessExtension::efpWidgetAccepted( const GamessEfpMatchDialog::Type &type,
                    const QString &name, const QList<QVector<Atom *> > &groups )
  {
    QString groupName = name;
    if ( !groupName.size() ) {
      groupName = tr( "Group Name" );
    }

    QList<QStandardItem *> newItems;
    foreach( QVector<Atom *> group, groups ) {
      QString groupString;
      bool first = true;


      foreach( Atom *atom, group ) {

        if ( !first ) {
          groupString.append( tr( ", " ) );
        } else {
          first = false;
        }
        groupString.append( QString::number( atom->index()+1 ) );

      }

      QStandardItem *item = new QStandardItem();
      item->setData( qVariantFromValue( group ) );
      item->setText( groupString );

      GamessEFPGroup * efpGroup = new GamessEFPGroup();
      efpGroup->name = groupName.toStdString();
      efpGroup->type = type ? GamessEFPGroup::QMType : GamessEFPGroup::EFPType ;
      efpGroup->atoms = group.toStdVector();

      m_inputData->EFP->AddGroup(efpGroup);

      newItems.append(item);
    }

    QStandardItem *rootItem = 0;

    // check for preexisting group
    int rowCount = m_efpModel->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
      QStandardItem *item = m_efpModel->item(i);
      if(item->text() == groupName)
      {
        rootItem = item;
        break;
      }
    }

    // if we have new items and there is no previous root item
    // we must create one
    if(newItems.size() && !rootItem)
    {
      rootItem = new QStandardItem(groupName);
      QStandardItem *typeItem  = new QStandardItem(tr(" (") + tr( type ? "qm" : "efp" ) + tr(")") );
      QList<QStandardItem *> row;
      row.append(rootItem);
      row.append(typeItem);

      m_efpModel->appendRow( row );
    }

    foreach(QStandardItem *item, newItems)
    {
      rootItem->appendRow(item);

      QModelIndex index = item->index();
      m_efpView->setFirstColumnSpanned(index.row(), index.parent(), true);
    }

    if ( m_gamessEfpDock ) {
      m_gamessEfpDock->show();
    }

  }

  void GamessExtension::efpWidgetDone()
  {
    GamessEfpMatchDialog *dialog = qobject_cast<GamessEfpMatchDialog *>( sender() );

    GLWidget *widget = m_dialogWidgets.value(dialog);
//    qDebug() << m_widgetSelected.value(widget);
    widget->clearSelected();
    widget->setSelected( m_widgetSelected.value( widget ), true );
    m_widgetSelected.remove(widget);

    m_dialogWidgets.remove(dialog);
    m_dialogMolecules.remove(dialog);

    // delete on close is set!
    if(dialog == m_efpDialog)
    {
      m_efpDialog = 0;
    }

    if(dialog == m_qmDialog)
    {
      m_qmDialog = 0;
    }

  }

  void GamessExtension::removePrimitive(Primitive *primitive)
  {
    if(primitive->type() == Primitive::AtomType)
    {
      m_inputData->EFP->RemoveGroups(static_cast<Atom *>(primitive));
    }

    for(int parentNum = 0; parentNum < m_efpModel->rowCount(); )
    {
      QStandardItem *parentItem = m_efpModel->item(parentNum);

      bool matched = false;
      for(int childNum = 0; childNum < parentItem->rowCount(); )
      {
        QStandardItem *childItem = parentItem->child(childNum);
        QVector<Atom *> atoms = childItem->data().value<QVector<Atom *> >();

        QString text;
        bool first = true;

        foreach(Atom *atom, atoms)
        {
          if ( !first ) {
            text.append( tr( ", " ) );
          } else {
            first = false;
          }
          text.append( QString::number( atom->index()+1 ) );

          if(atom == primitive)
          {
            parentItem->removeRow(childNum);
            matched = true;
          }
        }
        if(!matched)
        {
          childItem->setText(text);
          childNum++;
        }
        else
        {
          matched = false;
        }
      }
      if(parentItem->rowCount() == 0)
      {
        m_efpModel->removeRow(parentNum);
      }
      else
      {
        parentNum++;
      }
    }
  }

}

#include "gamessextension.moc"

Q_EXPORT_PLUGIN2(gamessextension, Avogadro::GamessExtensionFactory)
