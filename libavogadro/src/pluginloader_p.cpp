/**********************************************************************
  PluginLoader -

  Copyright (C) 2010 Konstantin Tokarev

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

#include "pluginloader_p.h"

#include <avogadro/pluginmanager.h>
#include "config.h" // krazy:exclude=includes

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QPluginLoader>
#include <QtCore/QSettings>

#include <QtGui/QMessageBox>

#include <QtCore/QDebug>

namespace Avogadro {

  /// @todo For 2.0: move late plugin loading to PluginItem
  class ProxyFactory : public PluginFactory
  {
  public:
    ProxyFactory(const PluginItem *item) : m_item(item), m_factory(0), m_triedToLoad(false) {}

    Plugin *createInstance(QObject *parent = 0)
    {
      if(!m_triedToLoad) {
        m_triedToLoad = true;
        qDebug() << "Loading" << m_item->absoluteFilePath();
        QPluginLoader loader(m_item->absoluteFilePath());
        QObject *instance = loader.instance();
        if (instance)
          m_factory = qobject_cast<PluginFactory *> (instance);
        else
          qDebug() << m_item->absoluteFilePath() << "failed to load. " << loader.errorString();
      }
      if(m_factory)
        return m_factory->createInstance(parent);
      else
        return 0;
    }

    Plugin::Type type() const { return (Plugin::Type) m_item->type(); }
    QString identifier() const { return m_item->identifier(); }
    QString name() const { return m_item->name(); }
    QString description() const { return m_item->description(); }

  private:
    const PluginItem *m_item;
    PluginFactory *m_factory;
    bool m_triedToLoad;
  };

  PluginLoader::PluginLoader() : m_checkEnabled(true),
      m_pluginInfo(QCoreApplication::organizationName(), "libavogadroPlugins")
  {
    //QSettings pluginInfo(QCoreApplication::organizationName(), "libavogadroPlugins");
    QFileInfo appFile(QCoreApplication::applicationFilePath());
    QString appCode = QString::number(qHash(appFile.canonicalPath()), 16);
    QString modified = QString::number(appFile.lastModified().toTime_t(), 16);
    /*if(pluginInfo.value(appCode + "/modified").toString() != modified)
      pluginInfo.setValue(appCode + "/modified", modified);
    else*/
   //   m_checkEnabled = false;
  }

  PluginLoader * PluginLoader::instance()
  {
    static PluginLoader *obj = 0;
    if (!obj)
      obj = new PluginLoader();
    return obj;
  }

  PluginItem * PluginLoader::loadItem(const QString & fileName)
  {
    qDebug() << fileName;
    PluginFactory *factory = 0;
    PluginItem *item = 0;
    bool isModified = false;
    bool isNew = false;
    bool isForbidden = false;
    //QSettings m_pluginInfo(QCoreApplication::organizationName(), "libavogadroPlugins");
    QFileInfo info(fileName);

    m_pluginInfo.beginGroup(QCoreApplication::applicationName());
    m_pluginInfo.beginGroup(QString::number( qHash(info.canonicalFilePath()), 16 ));

    isNew = m_pluginInfo.value("id").isNull();
    isForbidden = (m_pluginInfo.value("id") == "f");

    if (isNew && m_checkEnabled) {
//      isForbidden = (QMessageBox::warning(0, tr("New plugin found"), tr("Warning: new plugin")
//        + ' ' + fileName + ' ' + "was found! Do you want to enable it?",
//        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No);
    } else {
      isModified = (m_pluginInfo.value("modified").toString().toUInt(0, 16)
                    != info.lastModified().toTime_t());

      if (isModified && m_checkEnabled) {
//        isForbidden = (QMessageBox::warning(0, tr("Plugin was modified"), tr("Warning: plugin")
//          + ' ' + fileName + ' ' + "was modified! Do you want to enable it?",
//          QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No);
      }

      // b - broken, f - forbidden
      if (!isModified && m_pluginInfo.value("id") == "b")
        return 0;
    }

    if ((isNew || isModified) && !isForbidden) {
//      // Remember new timestamp to prevent questions on next start
//      m_pluginInfo.setValue("modified", QString::number( info.lastModified().toTime_t(), 16 ));
//      QPluginLoader loader(fileName);
//      QObject *instance = loader.instance();
//      if (instance) {
//        factory = qobject_cast<PluginFactory *> (instance);
//        if(factory) {
//          // Save plugin information
//          m_pluginInfo.setValue("id", factory->identifier());
//          m_pluginInfo.setValue("name", factory->name());
//          m_pluginInfo.setValue("type", factory->type());
//          m_pluginInfo.setValue("description", factory->description());
//        } else {
//          qDebug() << fileName << "failed to load: factory not found";
//          m_pluginInfo.setValue("id", 'b');
//        }
//      } else {
//        qDebug() << fileName << "failed to load. " << loader.errorString();
//        m_pluginInfo.setValue("id", 'b');
//      }
    } else {
      // Create proxy factory and load plugin information without launching plugin
      qDebug() << "restoring" << fileName;
      item = new PluginItem(
          m_pluginInfo.value("name").toString(),
          m_pluginInfo.value("id").toString(),
          m_pluginInfo.value("description").toString(),
          (Plugin::Type) m_pluginInfo.value("type").toInt(),
          info.fileName(),
          info.absoluteFilePath(),
          0);
      factory = new ProxyFactory(item);
      item->setFactory(factory);
      //m_proxyFactories.append(factory);
//      qDebug() << factory;
    }
    m_pluginInfo.endGroup();
    m_pluginInfo.endGroup();
    if (factory) {
      // create the PluginItem
      item = new PluginItem(factory->name(), factory->identifier(),
        factory->description(), factory->type(), info.fileName(),
        info.absoluteFilePath(), factory);
      item->setNew(isNew);
      item->setModified(isModified);
      return item;
    } else {
      return 0;
    }
  }

  void PluginLoader::registerPlugin(const QString & fileName)
  {
    QFileInfo info(fileName);
    // Remember new timestamp to prevent questions on next start
    m_pluginInfo.setValue("modified", QString::number( info.lastModified().toTime_t(), 16 ));
    QPluginLoader loader(fileName);
    PluginFactory *factory = 0;
    QObject *instance = loader.instance();
    if (instance) {
      factory = qobject_cast<PluginFactory *> (instance);
      if(factory) {
        // Save plugin information
        m_pluginInfo.setValue("id", factory->identifier());
        m_pluginInfo.setValue("name", factory->name());
        m_pluginInfo.setValue("type", factory->type());
        m_pluginInfo.setValue("description", factory->description());
      } else {
        qDebug() << fileName << "failed to load: factory not found";
        m_pluginInfo.setValue("id", 'b');
      }
    } else {
      qDebug() << fileName << "failed to load. " << loader.errorString();
      m_pluginInfo.setValue("id", 'b');
    }
  }
}

#include "pluginloader_p.moc"
