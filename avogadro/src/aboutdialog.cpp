/**********************************************************************
  Copyright (C) 2007 Carsten Niehaus <cniehaus@kde.org>

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
#include "aboutdialog.h"
#include "config.h"

#include <QtCore/QVector>
#include <QtGui/QMouseEvent>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPainterPath>
#include <QtGui/QStyleOption>

#include <avogadro/global.h>

namespace Avogadro {

  AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent
#ifdef Q_WS_MAC
        , Qt::Tool
#endif
        )
    {
      QGridLayout *layout = new QGridLayout(this);
      QLabel *label = new QLabel(this);
      QLabel *lbl = new QLabel(this);
      QPushButton *cmd = new QPushButton(tr("OK"), this);
      QPixmap logo = QPixmap(":/icons/avogadro.png");

      label->setPixmap(logo.scaled(256, 256,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));

      lbl->setWordWrap(true);
      lbl->setOpenExternalLinks(true);

      // Build up the SCM revision numbers if present
      QString scmVersion(SCM_REVISION);
      QString appVersion(VERSION);
      if (scmVersion.length())
        appVersion += " (Git revision: " + scmVersion + ')';
      QString libVersion(Library::version());
      if (Library::scmRevision().length())
        libVersion += " (Git revision: " + Library::scmRevision() + ')';

      lbl->setText(
          tr("<h3>%1</h3>"
            "<br/><br/>Application Version: %2"
            "<br/><br/>Library Version: %3"
            "<br/><br/>For more information check the <a href=\"http://avogadro.openmolecules.net/\">Avogadro homepage</a>."
            "<br/><br/>The program is provided AS IS with NO WARRANTY OF ANY KIND,"
            " INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A"
            " PARTICULAR PURPOSE.<br/>"
             ).arg(tr("Avogadro")).arg(appVersion).arg(libVersion));

      cmd->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      cmd->setDefault(true);
      connect(cmd, SIGNAL(clicked()),
          this, SLOT(reject()));

      layout->addWidget(label, 0, 0, 1, 1);
      layout->addWidget(lbl, 0, 1, 4, 4);
      layout->addWidget(cmd, 4, 2, 1, 1);
    }

  AboutDialog::~AboutDialog()
  {
  }
}
#include "aboutdialog.moc"
