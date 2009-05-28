/**********************************************************************
  CustomColor - Class for using arbitrary colors

  Copyright (C) 2009 Geoffrey R. Hutchison

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

#include "customcolor.h"

#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/colorbutton.h>

#include <QtPlugin>
#include <QHBoxLayout>
#include <QLabel>

using namespace Eigen;

namespace Avogadro {

  /// Constructor
  CustomColor::CustomColor(): m_settingsWidget(NULL)
  { }

  /// Destructor
  CustomColor::~CustomColor()
  {
    if (m_settingsWidget)
      m_settingsWidget->deleteLater();
  }

  void CustomColor::settingsWidgetDestroyed()
  {
    m_settingsWidget = 0;
  }

  QWidget *CustomColor::settingsWidget()
  {
    if (!m_settingsWidget) {
      m_settingsWidget = new QWidget();
      QHBoxLayout *layout = new QHBoxLayout(m_settingsWidget);
      QLabel *label = new QLabel(tr("Custom Color:"), m_settingsWidget);
      ColorButton *button = new ColorButton(m_settingsWidget);
      layout->addWidget(label);
      layout->addWidget(button);

      connect(button, SIGNAL(colorChanged(QColor)),
              this, SLOT(colorChanged(QColor)));
      connect(m_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
    }

    return m_settingsWidget;
  }

  void CustomColor::colorChanged(QColor newColor)
  {
    Color::set(newColor);
    emit changed();
  }

  void CustomColor::set(const Primitive *)
  {
    // handled by the custom coloring
  }

  void CustomColor::writeSettings(QSettings &settings) const
  {
    settings.setValue("customcolor", Color::color());
  }

  void CustomColor::readSettings(QSettings &settings)
  {
    Color::set(settings.value("customcolor", QColor(1,1,1,1)).value<QColor>());
  }

}

Q_EXPORT_PLUGIN2(customcolor, Avogadro::CustomColorFactory)

