/**********************************************************************
  ceviewoptionswidget.h

  Copyright (C) 2011 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#ifndef CEVIEWOPTIONSWIDGET_H
#define CEVIEWOPTIONSWIDGET_H

#include "ceabstractdockwidget.h"
#include "ui_ceviewoptionswidget.h"

class QColor;
class QColorDialog;

namespace Avogadro
{
  class CrystallographyExtension;
  class GLWidget;
  class Molecule;

  class CEViewOptionsWidget : public CEAbstractDockWidget
  {
    Q_OBJECT

  public:
    CEViewOptionsWidget(CrystallographyExtension *ext);
    ~CEViewOptionsWidget();

    GLWidget * glWidget() const {return m_glWidget;}
    Molecule * molecule() const {return m_molecule;}

    enum NumCellChoice {
      NCC_Invalid = -1,
      NCC_None = 0,
      NCC_One,
      NCC_All
    };

  public slots:
    void setGLWidget(GLWidget *g);
    void setMolecule(Molecule *m);

  protected slots:
    void makeMoleculeConnections();
    void disconnectMoleculeConnections();
    void updateRepeatCells();
    void updateCamera();
    void updateMillerPlane();
    void millerIndexChanged();
    void updateCellRenderOptions();
    void setDisplayAllAtomImages(bool b);
    void generateExtraAtomImages();
    void removeExtraAtomImages();
    void resetExtraAtomImages();

    // Color selection
    void selectCellColor();
    void previewColor(const QColor &);
    void acceptColor();
    void rejectColor();
    void cleanupColorDialog();

    void cellChanged();

    // used for floating state changes
    void updateLayout(bool isFloating);

    // used for docking area changes
    void updateLayout(Qt::DockWidgetArea area);

  protected:
    Ui::CEViewOptionsWidget ui;
    GLWidget *m_glWidget;
    Molecule *m_molecule;
    Molecule *m_extraImagesMolecule;
    Qt::DockWidgetArea m_currentArea;
    NumCellChoice m_ncc;
    QColorDialog *m_colorDialog;
    QColor *m_origColor;
    bool m_displayAllAtomImages;
  };

}

#endif
