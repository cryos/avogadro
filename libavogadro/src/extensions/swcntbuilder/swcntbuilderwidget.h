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

#ifndef SWCNTBUILDERDIALOG_H
#define SWCNTBUILDERDIALOG_H

#include <avogadro/dockwidget.h>

#include "ui_swcntbuilderwidget.h"

namespace SWCNTBuilder {

  class SWCNTBuilderWidget : public Avogadro::DockWidget
  {
    Q_OBJECT

  public:
    explicit SWCNTBuilderWidget(const QString &title,
                                QWidget *parent = 0, Qt::WindowFlags f = 0 );
    explicit SWCNTBuilderWidget(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    virtual ~SWCNTBuilderWidget();

    enum LengthUnit {
      LU_Periodic = 0,
      LU_Angstrom,
      LU_Bohr,
      LU_Nanometer,
      LU_Picometer
    };

    unsigned int getN();
    unsigned int getM();
    double getLength();
    bool getCap();
    bool getDBonds();
    bool getAutohide();
    LengthUnit getLengthUnit();

  public Q_SLOTS:
    void startBuilding();
    void endBuilding();
    void writeSettings();

  protected Q_SLOTS:
    void readSettings();

  Q_SIGNALS:
    void buildClicked();

  protected:
    void ctor();

  private:
    Ui::SWCNTBuilderWidget ui;
  };
}

#endif
