/**********************************************************************
  UpdateCheck - Check for Avogadro updates

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

#include "updatecheck.h"

#include "updatedialog.h"

#include <avogadro/global.h>

#include <QtGui/QMessageBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QSettings>
#include <QtCore/QDebug>

namespace Avogadro
{
  UpdateCheck * UpdateCheck::instance = NULL;

  UpdateCheck * UpdateCheck::getInstance(QObject* parent)
  {
    if (!instance) {
      instance = new UpdateCheck(parent);
      return instance;
    }
    else
      return instance;
  }

  UpdateCheck::UpdateCheck(QObject* parent) : QObject(parent), m_network(0),
      m_versionPrompted(0)
  {
  }

  UpdateCheck::~UpdateCheck()
  {
    delete m_versionPrompted;
  }

  void UpdateCheck::writeSettings(QSettings &settings) const
  {
    if (m_versionPrompted)
      settings.setValue("updateVersionPrompted", *m_versionPrompted);
  }

  void UpdateCheck::readSettings(QSettings &settings)
  {
    if (!m_versionPrompted)
      m_versionPrompted = new QString;

    *m_versionPrompted = settings.value("updateVersionPrompted",
                                        Library::version()).toString();
  }

  void UpdateCheck::checkForUpdates()
  {
    if (!m_network) {
      m_network = new QNetworkAccessManager(this);
      connect(m_network, SIGNAL(finished(QNetworkReply*)),
              this, SLOT(replyFinished(QNetworkReply*)));
    }

    // Check the URL for the latest unstable version of Avogadro
#ifdef AVOGADRO_STABLE
    m_network->get(QNetworkRequest(QUrl("http://avogadro.openmolecules.net/version.txt")));
#else
    m_network->get(QNetworkRequest(QUrl("http://avogadro.openmolecules.net/unstable.txt")));
#endif
  }

  void UpdateCheck::replyFinished(QNetworkReply *reply)
  {
    // Read in all the data
    if (!reply->isReadable()) {
      QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                           tr("Network Update Check Failed"),
                           tr("Network timeout or other error."));
      reply->deleteLater();
      return;
    }

    QString version, releaseNotes;
    bool newVersionAvailable = false;

    // reply->canReadLine() always returns false, so this seems to best approach
    QStringList lines = QString(reply->readAll()).split('\n');
    for(int i = 0; i < lines.size(); ++i) {
      if (lines[i] == "[Version]" && lines.size() > ++i) {
        version = lines[i];
        if (versionCompare(version))
          newVersionAvailable = true;
      }
      if (lines[i] == "[Release Notes]" && lines.size() > ++i) {
        // Right now just reading in the rest of the file as release notes
        for (int j = i-1; j >=0; --j)
          lines.removeAt(j);
        releaseNotes = lines.join("\n");
      }
    }

    if (newVersionAvailable) {
      QPointer<UpdateDialog> info = new UpdateDialog(qobject_cast<QWidget *>(parent()), releaseNotes);
      info->exec();
      delete info;
    }
    // Now we have warned the user, set this version as the prompted version
    *m_versionPrompted = version;

    // We are responsible for deleting the reply object
    reply->deleteLater();
  }

  bool UpdateCheck::versionCompare(const QString& newVersion)
  {
    QStringList newParts = newVersion.split('.');
    if (!m_versionPrompted)
      m_versionPrompted = new QString(Library::version());
    QStringList oldParts = m_versionPrompted->split('.');

    if (newParts.size() < 3 || oldParts.size() < 3) {
      qDebug() << "Error size of new or old version strings is too small:"
          << Library::version() << newVersion;
    }
    int parts = newParts.size() < oldParts.size() ? newParts.size()
                                                  : oldParts.size();

    // Now compare each part of the version - return on the first that is bigger
    for (int i = 0; i < parts; ++i) {
      if (newParts[i] > oldParts[i])
        return true;
      else if (newParts[i] < oldParts[i]) // Should never happen, good to check though
        return false;
    }
    return false;
  }

} // End namespace Avogadro

#include "updatecheck.moc"
