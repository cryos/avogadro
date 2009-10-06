/**********************************************************************
  DaltonExtension - Extension for generating Dalton input files

  Source code shamelessly copied from gaussianextension.cpp

 **********************************************************************/

#include "daltonextension.h"

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include <avogadro/molecule.h>

#include <QMessageBox>
#include <QFile>

using namespace OpenBabel;
using namespace std;

namespace Avogadro
{

  DaltonExtension::DaltonExtension(QObject* parent) : Extension(parent),
    m_daltonInputDialog(0), m_molecule(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("&Dalton Input..."));
    action->setData("Dalton");
    m_actions.append(action);
  }

  DaltonExtension::~DaltonExtension()
  {
    if(m_daltonInputDialog)
    {
      m_daltonInputDialog->close();
      m_daltonInputDialog->deleteLater();
    }
  }

  QList<QAction *> DaltonExtension::actions() const
  {
    return m_actions;
  }

  QString DaltonExtension::menuPath(QAction*) const
  {
    return tr("E&xtensions") + '>' + tr("&Dalton");
  }

  QUndoCommand* DaltonExtension::performAction(QAction *action, GLWidget *widget)
  {
    m_widget = widget;

    if (action->data() == "Dalton") {
      if (!m_daltonInputDialog) {
        m_daltonInputDialog = new DaltonInputDialog(static_cast<QWidget*>(parent()));
        m_daltonInputDialog->setMolecule(m_molecule);
        m_daltonInputDialog->show();
      }
      else
        m_daltonInputDialog->show();
    }
    return 0;
  }

  void DaltonExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
    if (m_daltonInputDialog)
      m_daltonInputDialog->setMolecule(m_molecule);
  }

  void DaltonExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
    if (m_daltonInputDialog) {
      m_daltonInputDialog->writeSettings(settings);
    }
  }

  void DaltonExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
    if (m_daltonInputDialog) {
      m_daltonInputDialog->readSettings(settings);
    }
    else {
      m_daltonInputDialog = new DaltonInputDialog(static_cast<QWidget*>(parent()));
      m_daltonInputDialog->readSettings(settings);
      if (m_molecule) {
        m_daltonInputDialog->setMolecule(m_molecule);
      }
    }
  }

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(daltonextension, Avogadro::DaltonExtensionFactory)

