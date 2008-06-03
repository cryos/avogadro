/**********************************************************************
  MacChemPasteboard.cpp - Mac pasteboard mapping for ChemDraw and other data

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

#include "macchempasteboard.h"
#include <QDebug>

#ifdef Q_WS_MAC
#include <Carbon/Carbon.h>
// global reference to the copy & paste pasteboard
PasteboardRef gClipboard, gFindPasteboard;
#endif

namespace Avogadro {
  
	const char* chemdrawMimeType = "chemical/x-cdx";

  class MacChemPasteboardPrivate
  {
  public:
#ifdef Q_WS_MAC
    MacChemPasteboardPrivate()
    {
      PasteboardCreate( kPasteboardClipboard, &inPasteboard );
      flavorType = NULL;
    }

    ~MacChemPasteboardPrivate()
    {
      if (inPasteboard != NULL)
        CFRelease(inPasteboard);
    }

    // A Mac pasteboard reference
    PasteboardRef inPasteboard;
    // save the flavor we find -- it's probably a dynamic type
    CFStringRef   flavorType;
#endif
  };

  MacChemPasteboard::MacChemPasteboard(): 
    QMacPasteboardMime(MIME_ALL|MIME_QT_CONVERTOR)
  {
    d = new MacChemPasteboardPrivate;
  }

  MacChemPasteboard::~MacChemPasteboard()
  {
    delete d;
  }
    
  QString MacChemPasteboard::mimeFor(QString flav)
  {
//    qDebug() << " mimeFor " << flav;
#ifdef Q_WS_MAC
    // We have to use some Apple's Carbon / CoreFoundation code
    // we want to look for ChemDraw data (os code "CDIF")
    CFArrayRef flavorTypeArray;
    CFIndex    flavorCount;
    PasteboardItemID itemID;
    OSStatus err = noErr;
    CFDataRef flavorDataSize;
    CFStringRef flavorType, osTypeFlavorType;
    char bytes[1024];

    // Get the current pasteboard
    PasteboardSynchronize(d->inPasteboard);
    err = PasteboardGetItemIdentifier(d->inPasteboard, 1, &itemID);
    if (err != noErr)
      return QString();

    // Find out what flavors are available
    err = PasteboardCopyItemFlavors(d->inPasteboard, itemID, &flavorTypeArray);
    if (err != noErr)
      return QString();
      
    flavorCount = CFArrayGetCount(flavorTypeArray);
    for (CFIndex flavorIndex = 0; flavorIndex < flavorCount; ++flavorIndex)
      {
        flavorType = (CFStringRef)CFArrayGetValueAtIndex(flavorTypeArray, flavorIndex);
        // Does this match the flavor requested by Qt?
        CFStringGetCString(flavorType, bytes, 1024, kCFStringEncodingMacRoman);
        if (flav != bytes) {
          continue; // Not the right flavor
        }

        // Check to see if we have the old 4-character OS type "swsC"
        osTypeFlavorType = UTTypeCopyPreferredTagWithClass(flavorType, kUTTagClassOSType);
        if (osTypeFlavorType == NULL)
          continue; // not the MDL molfile pasteboard

        CFComparisonResult result;
        result = CFStringCompare(osTypeFlavorType, CFSTR("CDIF"), 0);
        if (result == kCFCompareEqualTo) {
          d->flavorType = flavorType;
          return QString(chemdrawMimeType);
        }
      } // end for(flavors)
#endif
    return QString();
  }

  // Convert between the MIME type and Mac pasteboard flavor UTI
  // We'll save the dynamic UTI above, so we can confirm the conversion types
  QString MacChemPasteboard::flavorFor(const QString &mime)
  {
//		qDebug() << " flavorFor " << mime;
#ifndef Q_WS_MAC
    return QString();
#else
    if (mime != chemdrawMimeType)
      return QString();

    // We don't have a saved flavorType
    if (!d->flavorType || CFStringGetLength(d->flavorType) == 0)
      return QString();

    char bytes[1024];
    CFStringGetCString(d->flavorType, bytes, 1024, kCFStringEncodingMacRoman);
    return QString(bytes);
#endif
  }
  
  bool MacChemPasteboard::canConvert(const QString &mime, QString flav)
  { 
    if (mime != chemdrawMimeType) 
      return false;

    return mimeFor(flav) == mime;
  }
    
  QVariant MacChemPasteboard::convertToMime(QString const & mime,
                                            QList<QByteArray> data,
                                            QString flav)
  {
    // I wouldn't have any idea what to do if we get multiple entries
    if(data.count() > 1)
      qWarning("MacChemPasteboard: Cannot handle multiple member data");

    QByteArray item = data.first();
    //    qDebug() << " converting " << item.size() << " bytes";
    return item;
  }
    
  QList<QByteArray> MacChemPasteboard::convertFromMime(QString const & mime,
                                                       QVariant data,
                                                       QString flav)
  {
    QList<QByteArray> ret;
    ret.append(data.toByteArray());
    return ret;
  }

  MacChemPasteboard macMimeConverter; // global instance

} // end namespace

// end Q_WS_MAC
//#endif
