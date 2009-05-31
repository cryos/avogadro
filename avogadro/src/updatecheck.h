/**********************************************************************
  UpdateCheck - Check for Avogadro updates - only prompt once for a new version

  Copyright (C) 2009 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
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

#ifndef UPDATECHECK_H
#define UPDATECHECK_H

#include <QtCore/QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QString;
class QSettings;

namespace Avogadro
{
  class UpdateCheck : public QObject
  {
  Q_OBJECT

  public:
    UpdateCheck(QObject* parent = 0);
    virtual ~UpdateCheck();

    /**
     * Save settings.
     */
    void writeSettings(QSettings &settings) const;

    /**
     * Read settings.
     */
    void readSettings(QSettings &settings);

  public slots:
    /**
     * Check for available application updates.
     */
    void checkForUpdates();

  private:
    QNetworkAccessManager *m_network;
    QString               *m_versionPrompted;

    /**
     * Compare the two versions, passed in the form of x.y.z.
     * @param newVersion The new version to compare to the library version.
     * @return True if the new version is greater than the library version.
     */
    bool versionCompare(const QString& newVersion);

  private slots:
    void replyFinished(QNetworkReply*);

  };

} // End namespace Avogadro

#endif // UPDATECHECK_H
