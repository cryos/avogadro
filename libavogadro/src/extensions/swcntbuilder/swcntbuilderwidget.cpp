/**********************************************************************
  SWCNTBuilderDialog - Build single wall carbon nanotubes

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **********************************************************************/

#include "swcntbuilderwidget.h"

#include <avogadro/dockwidget.h>

#include <QtCore/QSettings>

#include <Eigen/Core>

using namespace Avogadro;

namespace SWCNTBuilder {

  SWCNTBuilderWidget::SWCNTBuilderWidget(const QString &title,
                                         QWidget *parent, Qt::WindowFlags f )
    : DockWidget( title, parent, f )
  {
    ctor();
  }

  SWCNTBuilderWidget::SWCNTBuilderWidget(QWidget *parent, Qt::WindowFlags f)
    : DockWidget (parent, f)
  {
    ctor();
  }

  SWCNTBuilderWidget::~SWCNTBuilderWidget()
  {
    this->writeSettings();
  }

  void SWCNTBuilderWidget::ctor()
  {
    ui.setupUi(this);

    this->readSettings();

    this->connect(ui.push_build, SIGNAL(clicked()), SIGNAL(buildClicked()));
  }

  unsigned int SWCNTBuilderWidget::getN()
  {
    return static_cast<unsigned int>(ui.spin_n->value());
  }

  unsigned int SWCNTBuilderWidget::getM()
  {
    return static_cast<unsigned int>(ui.spin_m->value());
  }

  unsigned int SWCNTBuilderWidget::getLength()
  {
    return static_cast<unsigned int>(ui.spin_length->value());
  }

  bool SWCNTBuilderWidget::getCap()
  {
    return ui.cb_cap->isChecked();
  }

  void SWCNTBuilderWidget::readSettings()
  {
    QSettings settings;
    settings.beginGroup("swcntbuilderextension");

    ui.spin_n->setValue(settings.value("n", 0).toInt());
    ui.spin_m->setValue(settings.value("m", 4).toInt());
    ui.spin_length->setValue(settings.value("length", 1).toInt());
    ui.cb_cap->setChecked(settings.value("cap", true).toBool());

    settings.endGroup();
  }

  void SWCNTBuilderWidget::writeSettings()
  {
    QSettings settings;
    settings.beginGroup("swcntbuilderextension");

    settings.setValue("n", this->getN());
    settings.setValue("m", this->getM());
    settings.setValue("length", this->getLength());
    settings.setValue("cap", this->getCap());

    settings.endGroup();
  }
}
