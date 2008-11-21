/**********************************************************************
  selectionitem.h - Base class for ProjectItem plugins.

  Copyright (C) 2008 by Tim Vandermeersch

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

#include "smartsitems.h"

#include <openbabel/parsmart.h>

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <QDebug>
#include <QString>
#include <QObject>

using namespace std;

namespace Avogadro
{

  SmartsItems::SmartsItems() : m_settingsWidget(0), m_label(0), m_widget(0)
  {
  }
 
  SmartsItems::~SmartsItems()
  {
    if (m_settingsWidget)
      delete m_settingsWidget;
  }
    
  void SmartsItems::setupModelData(GLWidget *widget, QTreeWidgetItem *parent)
  {
    // save the widget
    m_widget = widget;

    // add the label
    m_label = new QTreeWidgetItem(parent, ProjectItem::StaticType);
    m_label->setText(0, alias());

    Molecule *molecule = m_widget->molecule();
    disconnect(molecule, 0, this, 0);
    // connect some signals to keep track of changes
    connect(molecule, SIGNAL(primitiveAdded(Primitive*)), this, SLOT(primitiveSlot(Primitive*)));
    connect(molecule, SIGNAL(primitiveUpdated(Primitive*)), this, SLOT(primitiveSlot(Primitive*)));
    connect(molecule, SIGNAL(primitiveRemoved(Primitive*)), this, SLOT(primitiveSlot(Primitive*)));

    update();
  }

  void SmartsItems::update()
  {
    Molecule *molecule = m_widget->molecule();

    qDebug() << "-----------------------------------------";
    qDebug() << "SmartsItem::update() : alias = " << alias();
    qDebug() << "childCount = " << m_label->childCount();
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < m_label->childCount(); ++i) {
      QTreeWidgetItem *item = m_label->child(i);
      qDebug() << "    " << item->text(0);
      if (item->type() == ProjectItem::DynamicType) {
        qDebug() << "    -> delete";
        items << item;
        //m_label->removeChild(item);
        //delete item;
      }
    }
    foreach(QTreeWidgetItem *item, items) {
      delete item;
    }
    qDebug() << "-----------------------------------------";
//    qDeleteAll(m_label->takeChildren());

    OpenBabel::OBSmartsPattern smarts;
    smarts.Init(m_smarts.toStdString());
    OpenBabel::OBMol mol = molecule->OBMol();
    smarts.Match(mol);

    // if we have matches, add them
    if(smarts.NumMatches() != 0) {
      vector< vector <int> > mapList = smarts.GetUMapList();
      vector< vector <int> >::iterator i; // a set of matching atoms
      vector<int>::iterator j; // atom ids in each match
      
      int nmatch = 1;
      for (i = mapList.begin(); i != mapList.end(); ++i, ++nmatch) { // for each match
        // get the primitives (atoms)
        PrimitiveList primitives;
        for (j = i->begin(); j != i->end(); ++j) {
          primitives.append(molecule->atom(*j - 1));
        }
        
        ProjectItem *item = new ProjectItem(m_label, ProjectItem::DynamicType);
        item->setText(0, tr("match %1").arg(nmatch));
        item->setPrimitives(primitives);
      }

    } // end matches
  
  }
  
  void SmartsItems::primitiveSlot(Primitive*)
  {
    update();
  }
 
  QWidget *SmartsItems::settingsWidget()
  {
    if(!m_settingsWidget)
    {
      m_settingsWidget = new SmartsSettingsWidget();
      m_settingsWidget->smartsEdit->setText(m_smarts);
      connect(m_settingsWidget->smartsEdit, SIGNAL(textChanged(const QString&)), 
          this, SLOT(setSmarts(const QString&)));
    }

    return m_settingsWidget;
  }

  QString SmartsItems::smarts() const
  {
    return m_smarts;  
  }

  void SmartsItems::setSmarts(const QString &smarts)
  {
    m_smarts = smarts;
  }
  
  void SmartsItems::writeSettings(QSettings &settings) const
  {
    ProjectPlugin::writeSettings(settings);
    settings.setValue("smarts", smarts());
  }
  
  void SmartsItems::readSettings(QSettings &settings)
  {
    ProjectPlugin::readSettings(settings);
    setSmarts(settings.value("smarts", "").toString());
  }

} // end namespace Avogadro

#include "smartsitems.moc"

Q_EXPORT_PLUGIN2(smartsitems, Avogadro::SmartsItemsFactory)
