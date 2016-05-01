/**********************************************************************
VRMLDialog - Dialog to render images using POV-Ray

Copyright (C) 2008 Marcus D. Hanwell

This file is part of the Avogadro molecular editor project.
For more information, see <http://avogadro.cc/>

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

#include "VRMLdialog.h"

#include <QFileDialog>
#include <QString>
#include <QSettings>
#include <QDebug>

namespace Avogadro
{
	using Eigen::Vector3d;

	VRMLDialog::VRMLDialog(QWidget* parent, Qt::WindowFlags f)
		: QDialog(parent, f)
	{
		ui.setupUi(this);
		ui.doubleSpinBox->setValue(1.00); //default to scale of 1.0 instead of 0.0

		connect(ui.selectFileName, SIGNAL(clicked()), this, SLOT(selectFileName()));
		connect(ui.render, SIGNAL(clicked()), this, SLOT(renderClicked()));
		connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(calcSize()));
	}

	VRMLDialog::~VRMLDialog()
	{
	}

	void VRMLDialog::setFileName(const QString& fileName)
	{
		ui.fileName->setText(fileName);
	}

	QString VRMLDialog::fileName()
	{
		return ui.fileName->text();
	}

	double VRMLDialog::scale()
	{
		//figure out output size
		return ui.doubleSpinBox->value();
	}
	void VRMLDialog::selectFileName()
	{
		// Load a file
		QString fileName = QFileDialog::getSaveFileName(this,
			tr("Save VRML rendered files"), ui.fileName->text(),
			tr("VRML files (*wrl)"));
		ui.fileName->setText(fileName);
	}

	void VRMLDialog::renderClicked()
	{
		emit render();
	}

	void VRMLDialog::calcSize()
	{
		emit calc();
	}

	void VRMLDialog::resized()
	{
		
	}

	void VRMLDialog::calcVRML(double thinnestCyl, double smallestSphere) //may need other args
	{
		ui.label_4->setEnabled(true);
		ui.label_4->setText("Thinnest cyclinder (mm): " + QString::number(thinnestCyl) + " mm");

		ui.label_5->setEnabled(true);
		ui.label_5->setText("Smallest sphere (mm): " + QString::number(smallestSphere) + " mm");
	}
} // End namespace Avogadro

