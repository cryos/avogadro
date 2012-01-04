/**********************************************************************
  cetranslatewidget.cpp

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 ***********************************************************************/

#include "cetranslatewidget.h"

#include "../ceundo.h"

#include <avogadro/atom.h>
#include <avogadro/glwidget.h>
#include <avogadro/primitivelist.h>

namespace Avogadro
{
  CETranslateWidget::CETranslateWidget(CrystallographyExtension *ext,
                                       QMainWindow *w,
                                       GLWidget *gl)
    : CEAbstractDockWidget(ext, w),
      m_vector(0,0,0),
      m_gl(gl)
  {
    ui.setupUi(this);

    connect(ui.combo_translateMode, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateVector()));
    connect(ui.combo_units, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateVector()));
    connect(ui.spin_x, SIGNAL(valueChanged(double)),
            this, SLOT(updateVector()));
    connect(ui.spin_y, SIGNAL(valueChanged(double)),
            this, SLOT(updateVector()));
    connect(ui.spin_z, SIGNAL(valueChanged(double)),
            this, SLOT(updateVector()));

    connect(ui.push_go, SIGNAL(clicked()),
            this, SLOT(translate()));

    connect(&m_selectionTimer, SIGNAL(timeout()),
            this, SLOT(checkSelection()));

    connect(ui.combo_units, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateGui()));

    // Set error label to show red text
    ui.label_error->setStyleSheet("QLabel {color: red;}");
    ui.label_error->hide();

    readSettings();
  }

  CETranslateWidget::~CETranslateWidget()
  {
    writeSettings();
  }

  void CETranslateWidget::updateGui()
  {
    // Determine unit string
    QString lengthSuffix;
    switch (m_ext->lengthUnit()) {
    case Angstrom:
      lengthSuffix = " " + CE_ANGSTROM;
      break;
    case Bohr:
      lengthSuffix = " a" + CE_SUB_ZERO;
      break;
    case Nanometer:
      lengthSuffix = " nm";
      break;
    case Picometer:
      lengthSuffix = " pm";
      break;
    default:
      lengthSuffix = "";
      break;
    }

    ui.spin_x->blockSignals(true);
    ui.spin_y->blockSignals(true);
    ui.spin_z->blockSignals(true);

    ui.spin_x->setValue(m_vector.x());
    ui.spin_y->setValue(m_vector.y());
    ui.spin_z->setValue(m_vector.z());

    ui.spin_x->setSuffix(lengthSuffix);
    ui.spin_y->setSuffix(lengthSuffix);
    ui.spin_z->setSuffix(lengthSuffix);

    ui.spin_x->blockSignals(false);
    ui.spin_y->blockSignals(false);
    ui.spin_z->blockSignals(false);
  }

  void CETranslateWidget::updateVector()
  {
    // Check translation mode
    switch (static_cast<TranslateMode>
            (ui.combo_translateMode->currentIndex())) {

    // Translating by an arbitrary vector
    case TM_VECTOR:
      // Disable selection monitor
      m_selectionTimer.stop();

      // Reenable editing of vector
      enableVectorEditor();

      // Clear any lingering errors
      clearError();

      // Pull out the coordinates
      m_vector.x() = ui.spin_x->value();
      m_vector.y() = ui.spin_y->value();
      m_vector.z() = ui.spin_z->value();
      break;

    // If moving an atom the origin, check for a single selected atom
    case TM_ATOM:
      // Start selection monitor
      m_selectionTimer.start(500);

      // Disable editing of vector
      disableVectorEditor();

      checkSelection();
      break;
    }
  }

  void CETranslateWidget::translate()
  {
    // Fetch current info
    const QList<QString> symbols = m_ext->currentAtomicSymbols();
    QList<Eigen::Vector3d> coords;
    if (ui.combo_units->currentIndex() == Cartesian) {
      coords = m_ext->currentCartesianCoords();
    }
    else {
      coords = m_ext->currentFractionalCoords();
    }

    // Translate coords
    for (QList<Eigen::Vector3d>::iterator it = coords.begin(),
           it_end = coords.end(); it != it_end; ++it) {
      *it += m_vector;
    }

    CEUndoState before (m_ext);

    if (ui.combo_units->currentIndex() == Cartesian) {
      m_ext->setCurrentCartesianCoords(symbols, coords);
    }
    else {
      m_ext->setCurrentFractionalCoords(symbols, coords);
    }

    if (ui.cb_wrap->isChecked()) {
      m_ext->wrapAtomsToCell();
    }

    CEUndoState after (m_ext);

    m_ext->pushUndo(new CEUndoCommand (before, after,
                                       tr("Translate Atoms")));

  }

  void CETranslateWidget::checkSelection()
  {
    QList<Primitive*> atoms = m_gl->selectedPrimitives().subList
      (Primitive::AtomType);

    if (!atoms.size())
      return setError(tr("Please select one or more atoms."));

    clearError();

    // Calculate centroid of selection
    m_vector = Eigen::Vector3d::Zero();
    for (QList<Primitive*>::const_iterator
           it = atoms.constBegin(),
           it_end = atoms.constEnd();
         it != it_end; ++it) {
      m_vector += (*qobject_cast<Atom* const>(*it)->pos());
    }
    m_vector /= -static_cast<double>(atoms.size());

    updateGui();
  }

  void CETranslateWidget::disableVectorEditor()
  {
    ui.spin_x->setReadOnly(true);
    ui.spin_y->setReadOnly(true);
    ui.spin_z->setReadOnly(true);
  }

  void CETranslateWidget::enableVectorEditor()
  {
    ui.spin_x->setReadOnly(false);
    ui.spin_y->setReadOnly(false);
    ui.spin_z->setReadOnly(false);
  }

  void CETranslateWidget::setError(const QString &err)
  {
    ui.label_error->setText(err);
    ui.push_go->setEnabled(false);
    ui.label_error->show();
  }

  void CETranslateWidget::clearError()
  {
    ui.label_error->hide();
    ui.push_go->setEnabled(true);
  }

  void CETranslateWidget::readSettings()
  {
    // Just a placeholder for now
  }

  void CETranslateWidget::writeSettings()
  {
    // Just a placeholder for now
  }

}
