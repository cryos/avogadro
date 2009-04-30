/**********************************************************************
  NetworkFetchExtension - Extension for fetching molecules over the network

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef NETWORKFETCHEXTENSION_H
#define NETWORKFETCHEXTENSION_H

#include <avogadro/extension.h>

class QNetworkAccessManager;
class QNetworkReply;
class QString;

namespace Avogadro
{
  class NetworkFetchExtension : public Extension
  {
  Q_OBJECT
    AVOGADRO_EXTENSION("NetworkFetch", tr("Network Fetch"),
                       tr("Fetch molecule files over the network."))

  public:
    NetworkFetchExtension(QObject* parent = 0);
    virtual ~NetworkFetchExtension();

    /**
     * @return a list of actions which this widget can perform
     */
    virtual QList<QAction *> actions() const;

    /**
     * @return the menu path for the specified action
     */
    virtual QString menuPath(QAction* action) const;

    /**
     * @param action The action that triggered the calls.
     * @param widget The currently active GLWidget (feedback to the user).
     * @return An undo command for this action.
     */
    virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);

    /**
     * Save the settings for this extension.
     * @param settings Settings variable to write settings to.
     */
    virtual void writeSettings(QSettings &settings) const;

    /**
     * Read the settings for this extension.
     * @param settings Settings variable to read settings from.
     */
    virtual void readSettings(QSettings &settings);

  public slots:

    /**
     * Slot to change the current molecule.
     */
    void setMolecule(Molecule *molecule);

  private:
    GLWidget* m_glwidget;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    QNetworkAccessManager *m_network;
    QString *m_pdbName;

  private slots:
    void replyFinished(QNetworkReply*);

  };

  class NetworkFetchExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
    AVOGADRO_EXTENSION_FACTORY(NetworkFetchExtension)
  };

} // End namespace Avogadro

#endif // NETWORKFETCHEXTENSION_H
