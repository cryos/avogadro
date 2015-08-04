/**********************************************************************
  SymmetryExtension - Point group perception and symmetrization via libmsym

  Copyright (C) 2015 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.cc/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "symmetryextension.h"

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtCore/QString>
#include <QDebug>

#include <openbabel/mol.h>

using namespace OpenBabel;
using namespace msym;

namespace Avogadro {

#define DEFAULT_ZERO_THRESHOLD 1.0e-3
#define DEFAULT_GEOMETRY_THRESHOLD 1.0e-3
#define DEFAULT_ANGLE_THRESHOLD 1.0e-3
#define DEFAULT_EQUIVALENCE_THRESHOLD 5.0e-4
#define DEFAULT_EIGFACT_THRESHOLD 1.0e-3
#define DEFAULT_PERMUTATION_THRESHOLD 5.0e-3
#define DEFAULT_ORTHOGONALIZATION_THRESHOLD 0.1


  msym_thresholds_t tight_thresholds = { // all defaults
    .zero = 1.0e-3,
    .geometry = 1.0e-3,
    .angle = 1.0e-3,
    .equivalence = 5.0e-4,
    .permutation = 5.0e-3,
    .eigfact = 1.0e-3,
    .orthogonalization = 0.1
};

msym_thresholds_t medium_thresholds = {
    .zero = 1.0e-2,
    .geometry = 1.0e-2,
    .angle = 1.0e-2,
    .equivalence = 6.3e-3,
    .permutation = 1.58e-2,
    .eigfact = 1.0e-3,
    .orthogonalization = 0.1
};

msym_thresholds_t loose_thresholds = {
    .zero = 0.06,
    .geometry = 0.06,
    .angle = 0.06,
    .equivalence = 0.025,
    .permutation = 1.0e-1,
    .eigfact = 1.0e-3,
    .orthogonalization = 0.1
};

msym_thresholds_t sloppy_thresholds = {
    .zero = 0.08,
    .geometry = 0.1,
    .angle = 0.1,
    .equivalence = 0.06,
    .permutation = 1.0e-1,
    .eigfact = 1.0e-3,
    .orthogonalization = 0.1
};

  SymmetryExtension::SymmetryExtension(QObject *parent) : Extension(parent),
                                                          m_molecule(0), m_dialog(0),
                                                          m_tolerance(0)
  {
    QAction *action = new QAction(this);
    action->setText(tr("Symmetry Properties..."));
    m_actions.append(action);

    ctx = msymCreateContext();
  }

  SymmetryExtension::~SymmetryExtension()
  {
    if (ctx != NULL) {
      msymReleaseContext(ctx);
    }
    if (m_dialog)
      m_dialog->close();
  }

  QList<QAction *> SymmetryExtension::actions() const
  {
    return m_actions;
  }

  QString SymmetryExtension::menuPath(QAction *) const
  {
    return tr("&View") + '>' + tr("&Properties");
  }

  void SymmetryExtension::setMolecule(Molecule *molecule)
  {
    if (m_molecule)
      disconnect( m_molecule, 0, this, 0 );

    m_molecule = molecule;
    update();
  }

  QUndoCommand* SymmetryExtension::performAction(QAction *, GLWidget *widget)
  {
    if (!m_molecule)
      return 0; // nothing we can do

    // Disconnect in case we're attached to a new widget
    if (m_widget)
      disconnect( m_molecule, 0, this, 0 );

    if (widget) {
      connect(widget, SIGNAL(moleculeChanged(Molecule *)),
              this, SLOT(moleculeChanged(Molecule*)));
      m_widget = widget;

      constructDialog();
      update();
      m_dialog->show();
    }

    return 0;
  }

  void SymmetryExtension::update()
  {
    if (m_dialog == NULL || m_molecule == NULL)
      return;

    detectSymmetry();
  }

  QString SymmetryExtension::pgSymbol(char* point_group)
  {
    QString pointGroup(point_group);
    if (pointGroup.isEmpty())
      pointGroup = "C1"; // default

    if (m_molecule && m_molecule->numAtoms() == 1)
      pointGroup = "Kh"; // sphere

    // check if we need an infinity symbol
    if (pointGroup[1] == '0')
      pointGroup = pointGroup.replace(1,1,trUtf8("\u221e"));

    // After first character, point group should subscript everything
    pointGroup.insert(1, "<sub>");
    pointGroup.append("</sub>");

    return pointGroup;
  }

  void SymmetryExtension::detectSymmetry()
  {
    if (m_molecule == NULL)
      return;

    // interface with libmsym
    msym_error_t ret = MSYM_SUCCESS;
    msym_element_t *elements = NULL;
    const char *error = NULL;
    char point_group[6];
    double cm[3], radius = 0.0, symerr = 0.0;
    unsigned int length = m_molecule->numAtoms();

    /* Do not free these variables */
    msym_symmetry_operation_t *msops = NULL;
    msym_subgroup_t *msg = NULL;
    int msgl = 0, msopsl = 0, mlength = 0;

    // initialize the c-style array of atom names and coordinates
    msym_element_t *a;
    a = (msym_element_t *)malloc(length*sizeof(msym_element_t));
    memset(a,0,length*sizeof(msym_element_t));

    foreach (Atom *atom, m_molecule->atoms()) {
      unsigned int i = atom->index();
      a[i].n = atom->atomicNumber();
      a[i].v[0] = atom->pos()->x();
      a[i].v[1] = atom->pos()->y();
      a[i].v[2] = atom->pos()->z();
    }
    elements = a;

    if (ctx != NULL) {
      msymReleaseContext(ctx);
      ctx = msymCreateContext();
    }

    // Set the thresholds
    switch (m_dialog->toleranceCombo->currentIndex()) {
    case 3: // very loose
      msymSetThresholds(ctx, &sloppy_thresholds);
      break;
    case 2: // loose
      msymSetThresholds(ctx, &loose_thresholds);
      break;
    case 1: // normal
      msymSetThresholds(ctx, &medium_thresholds);
      break;
    case 0: // tight
    default:
      msymSetThresholds(ctx, &tight_thresholds);
    }

    if(MSYM_SUCCESS != (ret = msymSetElements(ctx, length, elements))) {
      free(elements);
      m_dialog->pointGroupText->setText(pgSymbol(0));
      return;
    }

    if(MSYM_SUCCESS != (ret = msymFindSymmetry(ctx))) {
      free(elements);
      m_dialog->pointGroupText->setText(pgSymbol(0));
      return;
    }

    /* Get the point group name */
    if(MSYM_SUCCESS != (ret = msymGetPointGroup(ctx, sizeof(char[6]), point_group))) {
      free(elements);
      m_dialog->pointGroupText->setText(pgSymbol(0));
      return;
    }

    // TODO: Subgroups
    //if(MSYM_SUCCESS != (ret = msymGetSubgroups(ctx, &msgl, &msg))) goto err;
    //    printf("Found point group [0] %s select subgroup\n",point_group);
    //for(int i = 0; i < msgl;i++) printf("\t [%d] %s\n",i+1,msg[i].name);

    if (m_dialog) {
      m_dialog->pointGroupText->setText(pgSymbol(point_group));
    }

    free(elements);
  }

  void SymmetryExtension::symmetrize()
  {
    msym_element_t *melements = NULL;
    int mlength = 0;
    double symerr = 0.0;
    msym_error_t ret = MSYM_SUCCESS;

    detectSymmetry();
    if(MSYM_SUCCESS != (ret = msymSymmetrizeMolecule(ctx, &symerr)))
      return;

    if(MSYM_SUCCESS != (ret = msymGetElements(ctx, &mlength, &melements)))
      return;

    // OK, now update our atoms
    foreach (Atom *atom, m_molecule->atoms()) {
      unsigned int i = atom->index();
      //      qDebug() << " before " << atom->pos()->x() << " " << atom->pos()->y() << " " << atom->pos()->z();
      atom->setPos(Eigen::Vector3d(melements[i].v));
      //      qDebug() << " after " << atom->pos()->x() << " " << atom->pos()->y() << " " << atom->pos()->z();
    }
  }

  void SymmetryExtension::updatePrimitives(Primitive*)
  {
  }

  void SymmetryExtension::updateAtoms(Atom*)
  {
  }

  void SymmetryExtension::updateBonds(Bond*)
  {
  }

  void SymmetryExtension::moleculeChanged(Molecule *)
  {
    update();
  }

  void SymmetryExtension::disableUpdating()
  {
    // don't ask for more updates
    disconnect( m_molecule, 0, this, 0 );
  }

  void SymmetryExtension::constructDialog()
  {
    if (!m_dialog) {
      m_dialog = new SymmetryDialog(m_widget);
      connect(m_dialog, SIGNAL(accepted()), this, SLOT(disableUpdating()));
      connect(m_dialog, SIGNAL(rejected()), this, SLOT(disableUpdating()));

      connect(m_dialog->detectSymmetryButton, SIGNAL(clicked()), this, SLOT(detectSymmetry()));
      connect(m_dialog->symmetrizeButton, SIGNAL(clicked()), this, SLOT(symmetrize()));

      m_dialog->toleranceCombo->setCurrentIndex(m_tolerance);
    }
  }

  void SymmetryExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
    m_tolerance = settings.value("tolerance", 1).toInt();
  }

  void SymmetryExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);

    if (!m_dialog)
      return; // nothing to save

    settings.setValue("tolerance", m_dialog->toleranceCombo->currentIndex());
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(symmetryextension,
                 Avogadro::SymmetryExtensionFactory)
