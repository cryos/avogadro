/**********************************************************************
  gl2psextension.cpp - Vector graphics export

  Copyright (C) 2010 by Konstantin Tokarev
  Based on code from mainwindow.cpp

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "gl2psextension.h"

#include <avogadro/glwidget.h>
#include <avogadro/molecule.h>

#include <QtCore/QStringList>
#include <QtCore/QDebug>

#include <QtGui/QAction>
#include <QtGui/QFileDialog>

// Include the GL2PS header
#include "gl2ps.h"

namespace Avogadro {

  Gl2psExtension::Gl2psExtension(QObject *parent) : Extension(parent)
  {
     m_actions.append(new QAction(tr("&Vector Graphics..."), this));
  }

  QString Gl2psExtension::menuPath(QAction *action) const
  {
    Q_UNUSED(action)
    return tr("&File") + '>' + tr("Export");
  }

  QList<QAction *> Gl2psExtension::actions() const
  {
    return m_actions;
  }

  QUndoCommand* Gl2psExtension::performAction(QAction *action, GLWidget *widget)
  {
    Q_UNUSED(action)
    QSettings settings;
    QString selectedFilter = settings.value("Export GL2PS Filter", tr("PDF")
                                            + " (*.pdf)").toString();
    QStringList filters;
    // Omit "common image formats" on Mac
    #ifdef Q_WS_MAC
      filters
    #else
      filters << tr("Common vector image formats")
        + " (*.pdf *.svg *.eps)"
    #endif
        << tr("All files") + " (* *.*)"
        << tr("PDF") + " (*.pdf)"
        << tr("SVG") + " (*.svg)"
        << tr("EPS") + " (*.eps)";

    // Use QFileInfo to get the parts of the path we want
    QFileInfo info(widget->molecule()->fileName());

    QString fileName = QFileDialog::getSaveFileName(widget,
                                       tr("Export Vector Graphics"),
                                       info.absolutePath(),
                                       filters.join(";;"),
                                       &selectedFilter);

    settings.setValue("Export GL2PS Filter", selectedFilter);

    if(fileName.isEmpty())
      return 0;

    qDebug() << "Exported filename:" << fileName;
    info.setFile(fileName);
    if(info.suffix().isEmpty()) {
      qDebug() << selectedFilter;
      if(selectedFilter.contains("pdf"))
        fileName += ".pdf";
      if(selectedFilter.contains("svg"))
        fileName += ".svg";
      if(selectedFilter.contains("eps"))
        fileName += ".eps";
      info.setFile(fileName);
    }
    qDebug() << info.fileName();

    // Just using the example right now, this is a C library but may be the
    // file calls need cleaning up a little.
    FILE *fp;
    int state = GL2PS_OVERFLOW, buffsize = 8*1024*1024, fileType = GL2PS_PDF;

    // Enumerate through the supported file types
    if (info.suffix() == "pdf")
      fileType = GL2PS_PDF;
    else if (info.suffix() == "svg")
      fileType = GL2PS_SVG;
    else if (info.suffix() == "eps")
      fileType = GL2PS_EPS;
    else
      return 0;

    fp = fopen(QFile::encodeName(fileName), "wb");
    qDebug() << "Writing out a vector graphics file...";
    while(state == GL2PS_OVERFLOW) {
      buffsize += 1024*1024;
      gl2psBeginPage("test", "gl2psTestSimple", NULL, fileType, GL2PS_BSP_SORT,
                     GL2PS_DRAW_BACKGROUND
                     | GL2PS_USE_CURRENT_VIEWPORT | GL2PS_OCCLUSION_CULL
                     | GL2PS_BEST_ROOT,
                     GL_RGBA, 0, NULL, 0, 0, 0, buffsize, fp,
                     info.baseName().toStdString().c_str());
      widget->renderNow();
      state = gl2psEndPage();
    }
    fclose(fp);
    qDebug() << "Done...";

    return 0;
  }

}

Q_EXPORT_PLUGIN2(gl2psextension, Avogadro::Gl2psExtensionFactory)
