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

#ifndef SWCNTBUILDEREXTENSION_H
#define SWCNTBUILDEREXTENSION_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>
#include <avogadro/extension.h>

class QThread;

// This is a work around for a bug on older versions of Avogadro, bug 3104853
using Avogadro::Plugin;

namespace SWCNTBuilder {
  class AvoTubeGen;
  class SWCNTBuilderWidget;

  class SWCNTBuilderExtension : public Avogadro::Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("SWCNT Builder",
                       tr("SWCNT Builder"),
                       tr("Construct single wall carbon nanotubes."))

  public:
    explicit SWCNTBuilderExtension(QObject *parent=0);
    virtual ~SWCNTBuilderExtension();

    virtual QList<QAction *> actions() const;
    virtual QString menuPath(QAction *action) const;
    virtual QUndoCommand* performAction(QAction *action,
                                        Avogadro::GLWidget *widget);
    virtual void setMolecule(Avogadro::Molecule *);

  public Q_SLOTS:
    void buildNanotube();
    void buildFinished();

  Q_SIGNALS:
    void requestBuild(uint, uint, bool, double, bool, bool);

  private:
    QList<QAction *> m_actions;
    Avogadro::Molecule *m_molecule;
    Avogadro::GLWidget *m_gl;
    SWCNTBuilderWidget *m_widget;
    QThread *m_thread;
    AvoTubeGen *m_tubegen;
  };

  // Plugin factory setup
  class SWCNTBuilderExtensionFactory
    : public QObject,
      public Avogadro::PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(SWCNTBuilderExtension)
  };

}

#endif

