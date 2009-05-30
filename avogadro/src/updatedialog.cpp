/**********************************************************************
  UpdateDialog - Dialog to display available Avogadro updates

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

#include "updatedialog.h"

#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>

namespace Avogadro {

  UpdateDialog::UpdateDialog(QWidget *parent, const QString &updateText)
    : QDialog(parent)
  {
    this->setWindowTitle(tr("Updated Version of Avogadro Available",
                            "Indicates an updated Avogadro version is available"));

    QLabel *mainText = new QLabel(this);
    QPushButton *okButton = new QPushButton(tr("OK"), this);

    // The main label displaying update information
    mainText->setWordWrap(true);
    // Want to have links opened if they are in the release notes
    mainText->setOpenExternalLinks(true);
    // Set the supplied text
    mainText->setText(updateText);

    okButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mainText);

    QHBoxLayout *okLayout = new QHBoxLayout(this);
    okLayout->addStretch();
    okLayout->addWidget(okButton);

    layout->addLayout(okLayout);
    setLayout(layout);
  }

  UpdateDialog::~UpdateDialog()
  {
  }
}

#include "updatedialog.moc"
