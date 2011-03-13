/**********************************************************************
  CBParameterEditor

  Copyright (C) 2011 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "cbparametereditor.h"

#include "../cbundo.h"
#include "../crystallographyextension.h"

namespace Avogadro
{
  CBParameterEditor::CBParameterEditor(CrystallographyExtension *ext,
                                       QMainWindow *w)
    : CBAbstractEditor(ext, w)
  {
    ui.setupUi(this);

    // Emit editStarted
    connect(ui.spin_a, SIGNAL(valueChanged(double)),
            this, SIGNAL(editStarted()));
    connect(ui.spin_b, SIGNAL(valueChanged(double)),
            this, SIGNAL(editStarted()));
    connect(ui.spin_c, SIGNAL(valueChanged(double)),
            this, SIGNAL(editStarted()));
    connect(ui.spin_alpha, SIGNAL(valueChanged(double)),
            this, SIGNAL(editStarted()));
    connect(ui.spin_beta, SIGNAL(valueChanged(double)),
            this, SIGNAL(editStarted()));
    connect(ui.spin_gamma, SIGNAL(valueChanged(double)),
            this, SIGNAL(editStarted()));

    // Apply button connections
    connect(ui.push_params_apply, SIGNAL(clicked()),
            this, SIGNAL(editAccepted()));
    connect(ui.push_params_apply, SIGNAL(clicked()),
            this, SLOT(setParameters()));

    // Reset button connections
    connect(ui.push_params_reset, SIGNAL(clicked()),
            this, SIGNAL(editRejected()));
    connect(ui.push_params_reset, SIGNAL(clicked()),
            this, SLOT(refreshEditor()));

    // Validation
    connect(ui.spin_a, SIGNAL(valueChanged(double)),
            this, SLOT(validateEditor()));
    connect(ui.spin_b, SIGNAL(valueChanged(double)),
            this, SLOT(validateEditor()));
    connect(ui.spin_c, SIGNAL(valueChanged(double)),
            this, SLOT(validateEditor()));
    connect(ui.spin_alpha, SIGNAL(valueChanged(double)),
            this, SLOT(validateEditor()));
    connect(ui.spin_beta, SIGNAL(valueChanged(double)),
            this, SLOT(validateEditor()));
    connect(ui.spin_gamma, SIGNAL(valueChanged(double)),
            this, SLOT(validateEditor()));

    // Apply/reset enable
    connect(ui.spin_a, SIGNAL(valueChanged(double)),
            this, SLOT(enableButtons()));
    connect(ui.spin_b, SIGNAL(valueChanged(double)),
            this, SLOT(enableButtons()));
    connect(ui.spin_c, SIGNAL(valueChanged(double)),
            this, SLOT(enableButtons()));
    connect(ui.spin_alpha, SIGNAL(valueChanged(double)),
            this, SLOT(enableButtons()));
    connect(ui.spin_beta, SIGNAL(valueChanged(double)),
            this, SLOT(enableButtons()));
    connect(ui.spin_gamma, SIGNAL(valueChanged(double)),
            this, SLOT(enableButtons()));
  }

  CBParameterEditor::~CBParameterEditor()
  {
  }

  void CBParameterEditor::refreshEditor()
  {
    CBUnitCellParameters params = m_ext->currentCellParameters();

    QString lengthSuffix;
    QString angleSuffix;
    switch (m_ext->lengthUnit()) {
    case Angstrom:
      lengthSuffix = " " + CB_ANGSTROM;
      break;
    case Bohr:
      lengthSuffix = " a" + CB_SUB_ZERO;
      break;
    default:
      lengthSuffix = "";
      break;
    }

    switch(m_ext->angleUnit()) {
    case Degree:
      angleSuffix = CB_DEGREE;
      break;
    case Radian:
      angleSuffix = " rad";
      break;
    default:
      angleSuffix = "";
      break;
    }

    ui.spin_a->blockSignals(true);
    ui.spin_b->blockSignals(true);
    ui.spin_c->blockSignals(true);
    ui.spin_alpha->blockSignals(true);
    ui.spin_beta->blockSignals(true);
    ui.spin_gamma->blockSignals(true);

    ui.spin_a->setValue(params.a);
    ui.spin_b->setValue(params.b);
    ui.spin_c->setValue(params.c);
    ui.spin_alpha->setValue(params.alpha);
    ui.spin_beta->setValue(params.beta);
    ui.spin_gamma->setValue(params.gamma);

    ui.spin_a->setSuffix(lengthSuffix);
    ui.spin_b->setSuffix(lengthSuffix);
    ui.spin_c->setSuffix(QString(lengthSuffix));
    ui.spin_alpha->setSuffix(QString(angleSuffix));
    ui.spin_beta->setSuffix(QString(angleSuffix));
    ui.spin_gamma->setSuffix(QString(angleSuffix));

    ui.spin_a->blockSignals(false);
    ui.spin_b->blockSignals(false);
    ui.spin_c->blockSignals(false);
    ui.spin_alpha->blockSignals(false);
    ui.spin_beta->blockSignals(false);
    ui.spin_gamma->blockSignals(false);

    ui.spin_a->setEnabled(true);
    ui.spin_b->setEnabled(true);
    ui.spin_c->setEnabled(true);
    ui.spin_alpha->setEnabled(true);
    ui.spin_beta->setEnabled(true);
    ui.spin_gamma->setEnabled(true);
    ui.push_params_apply->setEnabled(false);
    ui.push_params_reset->setEnabled(false);
    emit validInput();
  }

  void CBParameterEditor::lockEditor()
  {
    ui.spin_a->setEnabled(false);
    ui.spin_b->setEnabled(false);
    ui.spin_c->setEnabled(false);
    ui.spin_alpha->setEnabled(false);
    ui.spin_beta->setEnabled(false);
    ui.spin_gamma->setEnabled(false);
  }

  void CBParameterEditor::unlockEditor()
  {
    ui.spin_a->setEnabled(true);
    ui.spin_b->setEnabled(true);
    ui.spin_c->setEnabled(true);
    ui.spin_alpha->setEnabled(true);
    ui.spin_beta->setEnabled(true);
    ui.spin_gamma->setEnabled(true);
  }

  void CBParameterEditor::markAsInvalid()
  {
  }

  void CBParameterEditor::markAsValid()
  {
  }

  void CBParameterEditor::enableButtons()
  {
    ui.push_params_apply->setEnabled(true);
    ui.push_params_reset->setEnabled(true);
  }

  CBUnitCellParameters CBParameterEditor::validateEditor()
  {
    CBUnitCellParameters p;
    p.a = ui.spin_a->value();
    p.b = ui.spin_b->value();
    p.c = ui.spin_c->value();
    p.alpha = ui.spin_alpha->value();
    p.beta  = ui.spin_beta->value();
    p.gamma = ui.spin_gamma->value();
    emit validInput();
    return p;
  }

  void CBParameterEditor::setParameters()
  {
    CBUnitCellParameters p = validateEditor();
    // if (!p.isValid()) {
    //   return;
    // }
    CBUndoState before (m_ext);
    m_ext->setCurrentCellParameters(p);
    CBUndoState after (m_ext);
    m_ext->pushUndo(new CBUndoCommand (before, after,
                                       tr("Set Unit Cell Params")));
  }
}
