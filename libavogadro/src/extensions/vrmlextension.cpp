/**********************************************************************
VRMLExtension - Extension for generating VRML- rendered images

Copyright (C) 2008-2009 Marcus D. Hanwell

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

#include "vrmlextension.h"

#include "vrmlpainter.h"

#include <avogadro/molecule.h>
#include <avogadro/toolgroup.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QProcess>
#include <QFileInfo>
#include <QDebug>

namespace Avogadro
{

	VRMLExtension::VRMLExtension(QObject* parent) : Extension(parent),
		m_glwidget(0), m_VRMLDialog(0), m_molecule(0), m_process(0)
	{
		QAction* action = new QAction(this);
		action->setText(tr("VRML..."));
		m_actions.append(action);
	}

	VRMLExtension::~VRMLExtension()
	{
	}

	QList<QAction *> VRMLExtension::actions() const
	{
		return m_actions;
	}

	QString VRMLExtension::menuPath(QAction*) const
	{
		return tr("&File") + '>' + tr("Export");
	}

	QUndoCommand* VRMLExtension::performAction(QAction *, GLWidget *widget)
	{
		m_glwidget = widget;
		if (!m_VRMLDialog)
		{
			m_VRMLDialog = new VRMLDialog(static_cast<QWidget*>(parent()));
			connect(m_VRMLDialog, SIGNAL(render()),
				this, SLOT(render()));
			connect(m_VRMLDialog, SIGNAL(calc()),
				this, SLOT(calc()));
			QFileInfo info(m_molecule->fileName());
			m_VRMLDialog->setFileName(info.absolutePath() + '/' + info.baseName()
				+ ".wrl");
			calc();
			m_VRMLDialog->show();
		}
		else {
			QFileInfo info(m_molecule->fileName());
			m_VRMLDialog->setFileName(info.absolutePath() + '/' + info.baseName()
				+ ".wrl");
			calc();
			m_VRMLDialog->show();
		}
		return 0;
	}



	void VRMLExtension::setMolecule(Molecule *molecule)
	{
		m_molecule = molecule;
	}

	void VRMLExtension::calc()
	{
		m_VRMLDialog->calcVRML(-1, -2);
		double scale = m_VRMLDialog->scale();
		VRMLPainterDevice pd(QString(), m_glwidget, scale, m_VRMLDialog);
	}

	void VRMLExtension::render()
	{
		// Render the scene using VRML-
		QString fileName = m_VRMLDialog->fileName().mid(0,
			m_VRMLDialog->fileName().lastIndexOf("."));

		// Check a filename was supplied
		if (fileName.isEmpty()) {
			QMessageBox::warning(m_VRMLDialog, tr("No filename supplied."),
				tr("No valid filename was supplied."));
			return;
		}

		double scale = m_VRMLDialog->scale();

		QFile VRMLFile(fileName + ".wrl");
		if (VRMLFile.open(QIODevice::WriteOnly | QIODevice::Text)) {

			VRMLFile.close();
			VRMLPainterDevice pd(fileName + ".wrl", m_glwidget, scale, m_VRMLDialog);
		}
		else {
			QMessageBox::warning(m_VRMLDialog, tr("Cannot Write to File."),
				tr("Cannot write to file %1. Do you have permissions to write to that location?").arg(fileName + ".wrl"));
			return;
		}


	}

	void VRMLExtension::finished(int exitCode)
	{

		qDebug() << "Rendering complete.";
		QByteArray result = m_process->readAllStandardError();//m_process->readAll();
		qDebug() << "VRML- output:" << result << "Exit code:"
			<< exitCode;
		disconnect(m_process, 0, this, 0);
		m_process->deleteLater();
		m_process = 0;
	}

} // End namespace Avogadro

Q_EXPORT_PLUGIN2(VRMLextension, Avogadro::VRMLExtensionFactory)
