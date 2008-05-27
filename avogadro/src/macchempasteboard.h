/**********************************************************************
  MacChemPasteboard.h - Mac pasteboard mapping for ChemDraw and other data

  Copyright (C) 2008 by Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef MACCHEMPASTEBOARD_H
#define MACCHEMPASTEBOARD_H

//#ifdef Q_WS_MAC

#include <QMacPasteboardMime>
#include <QDebug>

namespace Avogadro {

  class MacChemPasteboardPrivate;

	/**
		* @class MacChemPasteboard
		*/
  class MacChemPasteboard : public QMacPasteboardMime
  {
    public:
    MacChemPasteboard();
    ~MacChemPasteboard();
    
    QString convertorName() { return QString("Chemistry"); }
    
    QString flavorFor(QString const & mime);
    
    QString mimeFor(QString flav);
    
    bool canConvert(const QString &mime, QString flav);
    
    QVariant convertToMime(const QString &mime, QList<QByteArray> data,
                           QString flav);
    
    QList<QByteArray> convertFromMime(const QString &mime,
                                      QVariant data,
                                      QString flav);

  private:
    MacChemPasteboardPrivate *d;
  };

  extern MacChemPasteboard macMimeConverter; // global instance

} // end namespace

// endif Q_WS_MAC
//#endif
// endif MACCHEMPASTEBOARD_H
#endif
