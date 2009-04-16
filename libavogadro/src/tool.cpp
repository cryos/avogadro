/**********************************************************************
  Tool - Avogadro Tool Interface

  Copyright (C) 2007 Donald Ephraim Curtis

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

#include "tool.h"
#include <config.h>

#include <QAction>
#include <QIcon>

namespace Avogadro {

  class ToolPrivate
  {
    public:
      ToolPrivate() {}
  };

  Tool::Tool(QObject *parent) : Plugin(parent), d(new ToolPrivate)
  {
    m_activateAction = new QAction(this);
    m_activateAction->setCheckable(true);
    m_activateAction->setIcon(QIcon(QString::fromUtf8(":/icons/tool.png")));
  }

  Tool::~Tool()
  {
    delete d;
  }

  Plugin::Type Tool::type() const
  {
    return Plugin::ToolType;
  }

  QString Tool::typeName() const
  {
    return tr("Tools");
  }

  QAction* Tool::activateAction() const {

    if(m_activateAction->toolTip().isEmpty())
      m_activateAction->setToolTip(description());

    if(m_activateAction->text().isEmpty())
      m_activateAction->setText(name());

    return m_activateAction;
  }

  QWidget* Tool::settingsWidget()
  {
    return 0;
  }

  void Tool::setMolecule(Molecule *)
  {
  }

  int Tool::usefulness() const
  {
    return 0;
  }

  bool Tool::operator<(const Tool &other) const {
    return usefulness() < other.usefulness();
  }

  QUndoCommand* Tool::wheelEvent(GLWidget *, QWheelEvent *)
  {
    return 0;
  }

  QUndoCommand* Tool::keyPressEvent(GLWidget *, QKeyEvent *)
  {
    return 0;
  }

  QUndoCommand* Tool::keyReleaseEvent(GLWidget *, QKeyEvent *)
  {
    return 0;
  }

  bool Tool::paint(GLWidget*)
  {
      return true;
  }

  void Tool::writeSettings(QSettings &settings) const
  {
    Q_UNUSED(settings);
  }

  void Tool::readSettings(QSettings &settings)
  {
    Q_UNUSED(settings);
  }

} // end namespace Avogadro

#include "tool.moc"
