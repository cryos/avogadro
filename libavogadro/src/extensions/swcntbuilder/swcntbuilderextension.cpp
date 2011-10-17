/**********************************************************************
  SWCNTBuilderExtension - Build single wall carbon nanotubes

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

#include "swcntbuilderextension.h"

#include "swcntbuilderwidget.h"
#include "avotubegen.h"

#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/molecule.h>

#include <openbabel/generic.h> // for OBUnitCell

#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>

using namespace Avogadro;

namespace SWCNTBuilder {

  SWCNTBuilderExtension::SWCNTBuilderExtension(QObject *parent)
    : Extension(parent),
      m_molecule(NULL),
      m_widget(NULL)
  {
    // Create the action
    QAction *action = new QAction(this);
    action->setText(tr("&Nanotube Builder"));
    m_actions.append(action);

    // Create the widget
    m_widget = new SWCNTBuilderWidget (tr("Nanotube Builder"));
    m_widget->setAllowedAreas(Qt::TopDockWidgetArea|Qt::BottomDockWidgetArea);
    m_widget->setPreferredDockWidgetArea(Qt::BottomDockWidgetArea);
    m_widget->hide();
    m_dockWidgets.append(m_widget);

    this->connect(m_widget, SIGNAL(buildClicked()), SLOT(addNanotube()));
  }

  SWCNTBuilderExtension::~SWCNTBuilderExtension()
  {
  }

  QList<QAction *> SWCNTBuilderExtension::actions() const
  {
    return m_actions;
  }

  QString SWCNTBuilderExtension::menuPath(QAction *) const
  {
    return tr("&Build");
  }

  QUndoCommand* SWCNTBuilderExtension::performAction( QAction *, GLWidget *)
  {
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

  void SWCNTBuilderExtension::addNanotube()
  {
    if (m_molecule == NULL) {
      qDebug() << Q_FUNC_INFO << "called with no molecule loaded?";
      return;
    }

    unsigned int n = m_widget->getN();
    unsigned int m = m_widget->getM();

    if (n < 2 && m < 2) {
      QMessageBox::critical(NULL, tr("Invalid Nanotube Specification"),
                            tr("The requested nanotube cannot be built "
                               "for the following reason:\n\n"
                               "Either n or m must be greater than 1."));
      return;
    }

    Molecule *nano = AvoTubeGen::getCarbonNanotube(n, m,
                                                   m_widget->getLength(),
                                                   m_widget->getCap());

    emit moleculeChanged(nano, Extension::NewWindow);

    m_widget->writeSettings();
    m_widget->hide();
  }

}

Q_EXPORT_PLUGIN2(swcntbuilderextension, SWCNTBuilder::SWCNTBuilderExtensionFactory)
