/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2016 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef YAEHMOP_EXTENSION_H
#define YAEHMOP_EXTENSION_H

#include <avogadro/extension.h>

namespace Avogadro {

  class YaehmopExtension : public Extension
  {
    Q_OBJECT
    AVOGADRO_EXTENSION("Yaehmop", tr("Yaehmop"),
                       tr("Yaehmop extension"))

  public:
    YaehmopExtension(QObject *parent=0);
    virtual ~YaehmopExtension();

    virtual QList<QAction *> actions() const;
    virtual QString menuPath(QAction *action) const;

    virtual void setMolecule(Molecule *molecule);

    virtual QDockWidget * dockWidget();
    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    // Write to Yaehmop and receive the output. This call is blocking (it will
    // not return until Yaehmop finishes). Returns true on success and false on
    // failure. The output is always written to QString output.
    bool executeYaehmop(QString input, QString& output) const;

  public slots:
    void calculateBandStructure() const;
    void plotTotalDOS() const;
    void plotPartialDOS() const;
    void setParametersFile();
    void executeCustomInput() const;

  private:
    // @param displayBandData This will be set to true if we are to
    // display the band data for the user.
    QString createYaehmopBandInput(bool& displayBandData) const;
    QString createGeometryAndLatticeInput() const;

    QList<QAction *> m_actions;
    Molecule *m_molecule;
    QString m_parametersFile;

  private Q_SLOTS:

  };

  class YaehmopExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
        AVOGADRO_EXTENSION_FACTORY(YaehmopExtension)
  };

} // end namespace Avogadro

#endif // YAEHMOP_EXTENSION_H
