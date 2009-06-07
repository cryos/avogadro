/**********************************************************************
  EngineTableView - custom engine table view for engine items

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef ENGINEVIEWWIDGET_H
#define ENGINEVIEWWIDGET_H

#include <QtGui/QWidget>

class QModelIndex;
class QTreeView;

namespace Avogadro {

  class GLWidget;
  class EngineItemModel;
  class Engine;
  /**
   * @class EngineViewWidget
   * @brief Widget for listing and editing Engines
   *
   * This widget lists the available engines of a GLWidget, allows for the
   * engines to be enabled or disabled, and emits a signal if the settings
   * dialog should be displayed for the engine.
   */
  class EngineViewWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit EngineViewWidget(GLWidget *glWidget, QWidget *parent = 0);
    ~EngineViewWidget();

    GLWidget * glWidget() const;

    /**
     * Clear function provided primarily for reloading plugins, i.e. engines.
     */
    void clear();

  private:
    GLWidget *m_glWidget;
    EngineItemModel *m_model;
    QTreeView *m_view;

  private slots:
    /**
     * Used to handle the mouse clicks on the view and to emit the right signal.
     */
    void selectEngine(const QModelIndex &index);

  signals:
    /**
     * Signal that the checkbox has been clicked - enable or disable it.
     */
    void clicked(Engine *);

    /**
     * Signal that the settings button has been clicked - the settings dialog
     * should be shown.
     */
    void settings(Engine *);
  };

}

#endif
