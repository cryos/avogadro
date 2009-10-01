/**********************************************************************
  DaltonExtension - Extension for generating Dalton input files

  Source code shamelessly copied from gaussianextension.h

 **********************************************************************/

#ifndef DALTONEXTENSION_H
#define DALTONEXTENSION_H

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

#include "daltoninputdialog.h"

namespace Avogadro
{
  class DaltonExtension : public Extension
  {
  Q_OBJECT
    AVOGADRO_EXTENSION("Dalton Input File Generator", tr("Dalton Input File Generator"), 
                       tr("Create input files for Dalton the quantum chemistry program"))
  public:
    DaltonExtension(QObject* parent = 0);
    virtual ~DaltonExtension();

    virtual QList<QAction *> actions() const;

    virtual QString menuPath(QAction* action) const;

    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    void setMolecule(Molecule *molecule);

    /**
     * Save the settings for this extension.
     * @param settings Settings variable to write settings to.
     */
    void writeSettings(QSettings &settings) const;

    /**
     * Read the settings for this extension.
     * @param settings Settings variable to read settings from.
     */
    void readSettings(QSettings &settings);
    
  private:
    DaltonInputDialog* m_daltonInputDialog;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    
    GLWidget *m_widget;
  };

  class DaltonExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(DaltonExtension) 
  };

} // End namespace Avogadro

#endif
