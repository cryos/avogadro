/**********************************************************************
  ProjectTreeEditor - Class for handling color changes in OpenGL

  Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
  Copyright (C) 2008 Tim Vandermeersch

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

#include <avogadro/projecttreeeditor.h>
#include <avogadro/pluginmanager.h>

#include <QDir>
#include <QIcon>
#include <QQueue>
#include <QHeaderView>

namespace Avogadro {

  ProjectTreeEditor::ProjectTreeEditor(QWidget *parent) : QDialog(parent), m_updating(false)
  {
    ui.setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QIcon upIcon(QString::fromUtf8("up.png"));
    QIcon downIcon(QString::fromUtf8("down.png"));
    QIcon backIcon(QString::fromUtf8("back.png"));
    QIcon forwardIcon(QString::fromUtf8("forward.png"));
    QIcon minusIcon(QString::fromUtf8("minus.png"));
    QIcon plusIcon(QString::fromUtf8("plus.png"));
    ui.newItemButton->setIcon(plusIcon);
    ui.deleteItemButton->setIcon(minusIcon);
    ui.moveItemUpButton->setIcon(upIcon);
    ui.moveItemDownButton->setIcon(downIcon);
    ui.moveItemRightButton->setIcon(forwardIcon);
    ui.moveItemLeftButton->setIcon(backIcon);

    ui.treeWidget->header()->setMovable(false);

    QList<PluginFactory*> factories = pluginManager.factories( Plugin::ProjectType );
    
    // add all possible types
    foreach(PluginFactory *factory, factories)
      ui.itemTypeCombo->addItem(factory->name());
  }

  ProjectTreeEditor::~ProjectTreeEditor()
  {
  }

  void ProjectTreeEditor::on_newItemButton_clicked()
  {
    m_updating = true;
    // create a new QTreeWidgetItem
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    QTreeWidgetItem *newItem = 0;
    if (curItem) {
        if (curItem->parent())
            newItem = new QTreeWidgetItem(curItem->parent(), curItem);
        else
            newItem = new QTreeWidgetItem(ui.treeWidget, curItem);
    } else
        newItem = new QTreeWidgetItem(ui.treeWidget);
    newItem->setText(0, ui.itemTypeCombo->currentText());
    
    // create a new ProjectPlugin for this QTreeWidgetItem
    PluginFactory *factory = pluginManager.factory(ui.itemTypeCombo->currentText(), Plugin::ProjectType);
    if (factory) 
    {
      ProjectPlugin *plugin = (ProjectPlugin*) factory->createInstance();
      m_hash[newItem] = plugin;
    }

    ui.treeWidget->setCurrentItem(newItem, 0);
    m_updating = false;
    updateEditor();
  }

  void ProjectTreeEditor::on_newSubItemButton_clicked()
  {
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    m_updating = true;

    // create a new QTreeWidgetItem
    QTreeWidgetItem *newItem = new QTreeWidgetItem(curItem);
    newItem->setText(0, ui.itemTypeCombo->currentText());

    // create a new ProjectPlugin for this QTreeWidgetItem
    PluginFactory *factory = pluginManager.factory(ui.itemTypeCombo->currentText(), Plugin::ProjectType);
    if (factory) 
    {
      ProjectPlugin *plugin = (ProjectPlugin*) factory->createInstance();
      m_hash[newItem] = plugin;
    }

    ui.treeWidget->setCurrentItem(newItem, 0);
    m_updating = false;
    updateEditor();
  }

  void ProjectTreeEditor::on_deleteItemButton_clicked()
  {
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    m_updating = true;

    QTreeWidgetItem *nextCurrent = 0;
    if (curItem->parent()) {
        int idx = curItem->parent()->indexOfChild(curItem);
        if (idx == curItem->parent()->childCount() - 1)
            idx--;
        else
            idx++;
        if (idx < 0)
            nextCurrent = curItem->parent();
        else
            nextCurrent = curItem->parent()->child(idx);
    } else {
        int idx = ui.treeWidget->indexOfTopLevelItem(curItem);
        if (idx == ui.treeWidget->topLevelItemCount() - 1)
            idx--;
        else
            idx++;
        if (idx >= 0)
            nextCurrent = ui.treeWidget->topLevelItem(idx);
    }
    closeEditors();
    delete curItem;

    if (nextCurrent)
        ui.treeWidget->setCurrentItem(nextCurrent, 0);

    m_updating = false;
    updateEditor();
  }

  void ProjectTreeEditor::on_moveItemUpButton_clicked()
  {
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    int idx;
    if (curItem->parent())
        idx = curItem->parent()->indexOfChild(curItem);
    else
        idx = ui.treeWidget->indexOfTopLevelItem(curItem);
    if (idx == 0)
        return;

    m_updating = true;

    QTreeWidgetItem *takenItem = 0;
    if (curItem->parent()) {
        QTreeWidgetItem *parentItem = curItem->parent();
        takenItem = parentItem->takeChild(idx);
        parentItem->insertChild(idx - 1, takenItem);
    } else {
        takenItem = ui.treeWidget->takeTopLevelItem(idx);
        ui.treeWidget->insertTopLevelItem(idx - 1, takenItem);
    }
    ui.treeWidget->setCurrentItem(takenItem, 0);

    m_updating = false;
    updateEditor();
  }

  void ProjectTreeEditor::on_moveItemDownButton_clicked()
  {
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    int idx, idxCount;
    if (curItem->parent()) {
        idx = curItem->parent()->indexOfChild(curItem);
        idxCount = curItem->parent()->childCount();
    } else {
        idx = ui.treeWidget->indexOfTopLevelItem(curItem);
        idxCount = ui.treeWidget->topLevelItemCount();
    }
    if (idx == idxCount - 1)
        return;

    m_updating = true;

    QTreeWidgetItem *takenItem = 0;
    if (curItem->parent()) {
        QTreeWidgetItem *parentItem = curItem->parent();
        takenItem = parentItem->takeChild(idx);
        parentItem->insertChild(idx + 1, takenItem);
    } else {
        takenItem = ui.treeWidget->takeTopLevelItem(idx);
        ui.treeWidget->insertTopLevelItem(idx + 1, takenItem);
    }
    ui.treeWidget->setCurrentItem(takenItem, 0);

    m_updating = false;
    updateEditor();
  }

  void ProjectTreeEditor::on_moveItemLeftButton_clicked()
  {
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    QTreeWidgetItem *parentItem = curItem->parent();
    if (!parentItem)
        return;

    m_updating = true;

    QTreeWidgetItem *takenItem = parentItem->takeChild(parentItem->indexOfChild(curItem));
    if (parentItem->parent()) {
        int idx = parentItem->parent()->indexOfChild(parentItem);
        parentItem->parent()->insertChild(idx, takenItem);
    } else {
        int idx = ui.treeWidget->indexOfTopLevelItem(parentItem);
        ui.treeWidget->insertTopLevelItem(idx, takenItem);
    }
    ui.treeWidget->setCurrentItem(takenItem, 0);

    m_updating = false;
    updateEditor();
  }

  void ProjectTreeEditor::on_moveItemRightButton_clicked()
  {
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    int idx, idxCount;
    if (curItem->parent()) {
        idx = curItem->parent()->indexOfChild(curItem);
        idxCount = curItem->parent()->childCount();
    } else {
        idx = ui.treeWidget->indexOfTopLevelItem(curItem);
        idxCount = ui.treeWidget->topLevelItemCount();
    }
    if (idx == idxCount - 1)
        return;

    m_updating = true;

    QTreeWidgetItem *takenItem = 0;
    if (curItem->parent()) {
        QTreeWidgetItem *parentItem = curItem->parent()->child(idx + 1);
        takenItem = curItem->parent()->takeChild(idx);
        parentItem->insertChild(0, takenItem);
    } else {
        QTreeWidgetItem *parentItem = ui.treeWidget->topLevelItem(idx + 1);
        takenItem = ui.treeWidget->takeTopLevelItem(idx);
        parentItem->insertChild(0, takenItem);
    }
    ui.treeWidget->setCurrentItem(takenItem, 0);

    m_updating = false;
    updateEditor();
  }

  void ProjectTreeEditor::on_treeWidget_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)
  {
    if (m_updating)
        return;
    updateEditor();
  }

  void ProjectTreeEditor::on_aliasEdit_textEdited(const QString &)
  {
    QTreeWidgetItem *current = ui.treeWidget->currentItem();  
    m_hash.value(current)->setAlias( ui.aliasEdit->text() );
  }
      
  void ProjectTreeEditor::on_settingsButton_clicked()
  {
    QTreeWidgetItem *current = ui.treeWidget->currentItem();
    m_hash.value(current)->settingsWidget()->show();
  }

  void ProjectTreeEditor::updateEditor()
  {
    QTreeWidgetItem *current = ui.treeWidget->currentItem();

    bool itemsEnabled = false;
    bool currentItemEnabled = false;
    bool moveItemUpEnabled = false;
    bool moveItemDownEnabled = false;
    bool moveItemRightEnabled = false;
    bool moveItemLeftEnabled = false;
    
    bool settingsButtonEnabled = false;
    QString aliasText, typeText;

    itemsEnabled = true;
    if (current) {
      int idx;
      int idxCount;
      
      currentItemEnabled = true;
      if (current->parent()) {
        moveItemLeftEnabled = true;
        idx = current->parent()->indexOfChild(current);
        idxCount = current->parent()->childCount();
      } else {
        idx = ui.treeWidget->indexOfTopLevelItem(current);
        idxCount = ui.treeWidget->topLevelItemCount();
      }
      
      if (idx > 0)
        moveItemUpEnabled = true;
      
      if (idx < idxCount - 1) {
        moveItemDownEnabled = true;
        moveItemRightEnabled = true;
      }

      ProjectPlugin *plugin = m_hash.value(current);
      if (plugin)
      {
        aliasText = plugin->alias();
        typeText = plugin->name();
        if (plugin->settingsWidget())
          settingsButtonEnabled = true;
      }

    }
    
    ui.itemsBox->setEnabled(itemsEnabled);
    ui.textLabel->setEnabled(currentItemEnabled);
    ui.itemTypeCombo->setEnabled(currentItemEnabled);
    ui.newSubItemButton->setEnabled(currentItemEnabled);
    ui.deleteItemButton->setEnabled(currentItemEnabled);

    ui.moveItemUpButton->setEnabled(moveItemUpEnabled);
    ui.moveItemDownButton->setEnabled(moveItemDownEnabled);
    ui.moveItemRightButton->setEnabled(moveItemRightEnabled);
    ui.moveItemLeftButton->setEnabled(moveItemLeftEnabled);
    
    ui.settingsButton->setEnabled(settingsButtonEnabled);
    ui.aliasEdit->setText( aliasText );
    ui.typeEdit->setText( typeText );
 
  }

  void ProjectTreeEditor::on_itemTypeCombo_currentIndexChanged(int)
  {
  /*
    if (m_updating)
        return;
    QTreeWidgetItem *curItem = ui.treeWidget->currentItem();
    if (!curItem)
        return;

    m_updating = true;
    curItem->setText(0, ui.itemTypeCombo->currentText());
    
    ProjectPlugin *oldPlugin = m_hash.value(curItem);
    delete oldPlugin;

    m_hash.remove(curItem);
    PluginFactory *factory = pluginManager.projectItemClassFactory().value(ui.itemTypeCombo->currentText());
    if (factory) 
    {
      ProjectPlugin *plugin = factory->createInstance();
      m_hash[curItem] = plugin;
    }

    m_updating = false;
    updateEditor();
    */
  }

  void ProjectTreeEditor::closeEditors()
  {
   
    if (QTreeWidgetItem *cur = ui.treeWidget->currentItem() ) {
      const int numCols = cur->columnCount ();
      for (int i = 0; i < numCols; i++) { 
        ui.treeWidget->closePersistentEditor (cur, i);
      }
    }
  }

  void ProjectTreeEditor::showEvent(QShowEvent*)
  {
    readSettings();
  }
  
  void ProjectTreeEditor::readSettings()
  {
    ui.treeWidget->clear();
    QTreeWidgetItem *newItem;
    QList<QTreeWidgetItem*> parents;
    QList<int> indentations;
    parents << ui.treeWidget->invisibleRootItem();
    indentations << 0;
    
    QSettings settings;
    settings.beginGroup("projectTree");
    int size = settings.beginReadArray("items");
    
    for (int i = 0; i < size; ++i) {
      settings.setArrayIndex( i );
      int position = settings.value("indent").toInt();

      if (position > indentations.last()) {
        // The last child of the current parent is now the new parent

        // unless the current parent has no children.
        if (parents.last()->childCount() > 0) {
          parents << parents.last()->child(parents.last()->childCount()-1);
            indentations << position;
        }
      } else {
        while (position < indentations.last() && parents.count() > 0) {
          parents.pop_back();
          indentations.pop_back();
        }
      }
      
      // Append a new item to the current parent's list of children.
      newItem = new QTreeWidgetItem(parents.last());
      newItem->setText(0, settings.value("name").toString());

      // create a new ProjectPlugin for this QTreeWidgetItem
      PluginFactory *factory = pluginManager.factory(settings.value("name").toString(), Plugin::ProjectType);
      if (factory) 
      {
        ProjectPlugin *plugin = (ProjectPlugin*) factory->createInstance();
        m_hash[newItem] = plugin;
        plugin->readSettings(settings);
      }

    }

    settings.endArray();
    settings.endGroup();  
  }
  
  void ProjectTreeEditor::writeItem(QSettings &settings, QTreeWidgetItem *cur, int indent, int &idx)
  {
    settings.setArrayIndex(idx);
    settings.setValue("indent", indent);
    m_hash.value(cur)->writeSettings(settings);

    indent++;
    idx++;

    for (int j = 0; j < cur->childCount(); ++j) {
      QTreeWidgetItem *child = cur->child( j );
      writeItem(settings, child, indent, idx);
    }
  }
  
  void ProjectTreeEditor::accept() 
  {
    int indent = 0;
    int idx = 0;
 
    QSettings settings;
    settings.beginGroup("projectTree");
    settings.beginWriteArray("items");

    for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i) { // for each top item
      QTreeWidgetItem *cur = ui.treeWidget->topLevelItem( i );
      writeItem(settings, cur, indent, idx);
    }

    settings.endArray();
    settings.endGroup();

    //deleteLater();
    hide();
  }

}

#include "projecttreeeditor.moc"
