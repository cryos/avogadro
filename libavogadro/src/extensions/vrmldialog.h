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

#ifndef VRMLDIALOG_H
#define VRMLDIALOG_H

#include "config.h"

#include <QDialog>
#include <QList>

#include <Eigen/Core>

#include "ui_vrmldialog.h"

class QSettings;

namespace Avogadro
{
	class VRMLDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit VRMLDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
		~VRMLDialog();



		/**
		* Set the file name of the POV-Ray file
		*/
		void setFileName(const QString& fileName);

		/**
		* @return The file name of the output.
		*/
		QString fileName();

		double scale();

	private:
		Ui::VRMLDialog ui;

	Q_SIGNALS:
		void render();
		void calc();
		void preview(int, int);

		public Q_SLOTS:
		void selectFileName();
		void renderClicked();
		void calcSize();
		void resized();
		void calcVRML(double shortestCyl, double smallestSphere);
	};

} // End namespace Avogadro

#endif
