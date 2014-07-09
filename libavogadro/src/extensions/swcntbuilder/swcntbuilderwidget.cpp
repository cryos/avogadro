/**********************************************************************
  SWCNTBuilderDialog - Build single wall carbon nanotubes

  Copyright (C) 2011 David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **********************************************************************/

#include "config.h"

#include "swcntbuilderwidget.h"

#include <avogadro/dockwidget.h>

#include <QtCore/QSettings>
#include <QtGui/QApplication>

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
    this->connect(ui.push_hide, SIGNAL(clicked()), SLOT(close()));
  }

  unsigned int SWCNTBuilderWidget::getN()
  {
    return static_cast<unsigned int>(ui.spin_n->value());
  }

  unsigned int SWCNTBuilderWidget::getM()
  {
    return static_cast<unsigned int>(ui.spin_m->value());
  }

  double SWCNTBuilderWidget::getLength()
  {
    return ui.spin_length->value();
  }

  SWCNTBuilderWidget::LengthUnit SWCNTBuilderWidget::getLengthUnit()
  {
    return static_cast<SWCNTBuilderWidget::LengthUnit>(
          ui.combo_lengthUnit->currentIndex());
  }

  bool SWCNTBuilderWidget::getCap()
  {
    return ui.cb_cap->isChecked();
  }

  bool SWCNTBuilderWidget::getDBonds()
  {
    return ui.cb_dbonds->isChecked();
  }

  bool SWCNTBuilderWidget::getAutohide()
  {
    return ui.push_autohide->isChecked();
  }

  void SWCNTBuilderWidget::startBuilding()
  {
    ui.push_build->setText(tr("Building..."));
    ui.push_build->setDisabled(true);
    qApp->setOverrideCursor(Qt::WaitCursor);

    ui.gb_nm->setDisabled(true);
    ui.gb_length->setDisabled(true);
    ui.gb_options->setDisabled(true);
  }

  void SWCNTBuilderWidget::endBuilding()
  {
    ui.push_build->setText(tr("&Build"));
    ui.push_build->setEnabled(true);
    qApp->restoreOverrideCursor();

    ui.gb_nm->setEnabled(true);
    ui.gb_length->setEnabled(true);
    ui.gb_options->setEnabled(true);
  }

  void SWCNTBuilderWidget::writeSettings()
  {
    QSettings settings;
    settings.beginGroup("swcntbuilderextension");

    settings.setValue("n", this->getN());
    settings.setValue("m", this->getM());
    settings.setValue("length", this->getLength());
    settings.setValue("lengthUnit", this->getLengthUnit());
    settings.setValue("cap", this->getCap());
    settings.setValue("dbonds", this->getDBonds());
    settings.setValue("autohide", this->getAutohide());

    settings.endGroup();
  }

  void SWCNTBuilderWidget::readSettings()
  {
    QSettings settings;
    settings.beginGroup("swcntbuilderextension");

    ui.spin_n->setValue(settings.value("n", 0).toInt());
    ui.spin_m->setValue(settings.value("m", 4).toInt());
    ui.spin_length->setValue(settings.value("length", 1.0).toDouble());
    ui.combo_lengthUnit->setCurrentIndex(
          settings.value("lengthUnit", LU_Periodic).toInt());
    ui.cb_cap->setChecked(settings.value("cap", true).toBool());
    ui.cb_dbonds->setChecked(settings.value("dbonds", false).toBool());
    ui.push_autohide->setChecked(settings.value("autohide", true).toBool());

    settings.endGroup();
  }
}
