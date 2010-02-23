/**********************************************************************
  PythonExtension - PythonExtension 

  Copyright (C) 2008 by Donald Ephraim Curtis
  Copyright (C) 2008,2009 by Tim Vandermeersch

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
 ***********************************************************************/

#ifndef PYTHONEXTENSION_H
#define PYTHONEXTENSION_H

#include <avogadro/extension.h>
#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>
#include <boost/python.hpp>

#include <QWidget>
#include <QList>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QFileInfo>
#include <QHash>

namespace Avogadro {

  class PythonScript;

  class PythonExtension : public Extension
  {
    Q_OBJECT

    public:
      //! Constructor
      explicit PythonExtension(QObject *parent = 0, const QString &filename = QString(""));
      //! Deconstructor
      virtual ~PythonExtension();

      //! @name To python delegated functions
      //@{
      QString identifier() const;
      QString name() const;
      QString description() const;
      QList<QAction *> actions() const;
      QString menuPath(QAction *action) const;
      QUndoCommand* performAction(QAction *action, GLWidget *widget);
      QDockWidget* dockWidget();
      bool paint(GLWidget *widget);
      void writeSettings(QSettings &settings) const;
      void readSettings(QSettings &settings);
      //@}

    public Q_SLOTS:
      void dockWidgetDestroyed();

    private:
      void loadScript(const QString &filename);

      PythonScript          *m_script;
      boost::python::object  m_instance;
      QDockWidget           *m_dockWidget;
      QString                m_identifier;
  };

  class PythonExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)

    public:
      PythonExtensionFactory(const QString &filename) : m_filename(filename)
      {
        PythonExtension extension(0, filename);
        m_identifier = extension.identifier();
        m_name = extension.name();
        m_desc = extension.description();
      }
      Plugin* createInstance(QObject *parent = 0)
      {
        return new PythonExtension(parent, m_filename);
      }
      Plugin::Type type() const { return Plugin::ExtensionType; }
      QString identifier() const { return m_identifier; }
      QString name() const { return m_name; }
      QString description() const { return m_desc; }
    private:
      QString m_filename;
      QString m_identifier, m_name, m_desc;
  };

} // end namespace Avogadro

#endif
