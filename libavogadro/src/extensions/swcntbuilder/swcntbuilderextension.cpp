/**********************************************************************
  SWCNTBuilderExtension - Build single wall carbon nanotubes

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

#include "swcntbuilderextension.h"

#include "swcntbuilderwidget.h"
#include "avotubegen.h"

#include "../insertcommand.h"

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

#include <openbabel/generic.h> // for OBUnitCell

#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>

using namespace Avogadro;

/// @todo Treat AvoTubeGen as a state machine to avoid 6 arg signals

namespace SWCNTBuilder {

  SWCNTBuilderExtension::SWCNTBuilderExtension(QObject *parent)
    : Extension(parent),
      m_molecule(NULL),
      m_widget(NULL),
      m_thread(new QThread ()),
      m_tubegen(new AvoTubeGen ())
  {
    // Move the tubegen into the background thread:
    m_thread->start();
    m_tubegen->moveToThread(m_thread);

    connect(this, SIGNAL(requestBuild(uint,uint,bool,double,bool,bool)),
            m_tubegen, SLOT(buildCarbonNanotube(uint,uint,bool,double,bool,bool)),
            Qt::QueuedConnection);
    this->connect(m_tubegen, SIGNAL(buildFinished()), SLOT(buildFinished()),
                  Qt::QueuedConnection);

    // Create the action
    QAction *action = new QAction(this);
    action->setText(tr("&Nanotube Builder..."));
    m_actions.append(action);

    // Create the widget
    m_widget = new SWCNTBuilderWidget (tr("Nanotube Builder"));
    m_widget->setAllowedAreas(Qt::TopDockWidgetArea|Qt::BottomDockWidgetArea);
    m_widget->setPreferredDockWidgetArea(Qt::BottomDockWidgetArea);
    m_widget->hide();
    m_dockWidgets.append(m_widget);

    this->connect(m_widget, SIGNAL(buildClicked()), SLOT(buildNanotube()));
    connect(this, SIGNAL(requestBuild(uint,uint,bool,double,bool,bool)),
            m_widget, SLOT(startBuilding()),
            Qt::DirectConnection);
    connect(m_tubegen, SIGNAL(buildFinished()),
            m_widget, SLOT(endBuilding()),
            Qt::QueuedConnection);
  }

  SWCNTBuilderExtension::~SWCNTBuilderExtension()
  {
    m_thread->terminate();
    m_thread->wait();
    delete m_thread;
  }

  QList<QAction *> SWCNTBuilderExtension::actions() const
  {
    return m_actions;
  }

  QString SWCNTBuilderExtension::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  QUndoCommand* SWCNTBuilderExtension::performAction( QAction *,
                                                      GLWidget *gl)
  {
    m_gl = gl;
    m_widget->show();
    return NULL;
  }

  void SWCNTBuilderExtension::setMolecule(Avogadro::Molecule *mol)
  {
    if (mol == m_molecule) {
      return;
    }

    m_molecule = mol;
  }

  void SWCNTBuilderExtension::buildNanotube()
  {
    unsigned int n = m_widget->getN();
    unsigned int m = m_widget->getM();

    if (n < 2 && m < 2) {
      QMessageBox::critical(NULL, tr("Invalid Nanotube Specification"),
                            tr("The requested nanotube cannot be built "
                               "for the following reason:\n\n"
                               "Either n or m must be greater than 1."));
      return;
    }

    bool cap = m_widget->getCap();
    bool dbonds = m_widget->getDBonds();

    // Clean up length units
    bool generatePeriodicCell = false;
    double length = m_widget->getLength();

    switch (m_widget->getLengthUnit()) {
    default:
      qWarning() << "Unrecognized length unit index:"
                 << m_widget->getLengthUnit();
      // Generate single, uncapped, periodic unit
      length = 1.0;
      generatePeriodicCell = true;
      break;
    case SWCNTBuilderWidget::LU_Periodic:
      generatePeriodicCell = true;
      break;
    case SWCNTBuilderWidget::LU_Angstrom:
      break;
    case SWCNTBuilderWidget::LU_Bohr:
      length *= 0.52918;
      break;
    case SWCNTBuilderWidget::LU_Picometer:
      length *= 1e-2;
      break;
    case SWCNTBuilderWidget::LU_Nanometer:
      length *= 1e1;
      break;
    }

    emit requestBuild(n, m, generatePeriodicCell, length, cap, dbonds);
  }

  void SWCNTBuilderExtension::buildFinished()
  {
    Avogadro::Molecule *nano = m_tubegen->getMolecule();

    emit performCommand(new InsertFragmentCommand(m_molecule, *nano, m_gl,
                                                  tr ("Insert Nanotube")));

    m_widget->writeSettings();
    if (m_widget->getAutohide()) {
      m_widget->hide();
    }
  }

}

Q_EXPORT_PLUGIN2(swcntbuilderextension, SWCNTBuilder::SWCNTBuilderExtensionFactory)
