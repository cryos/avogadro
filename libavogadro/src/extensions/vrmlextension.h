/**********************************************************************
VRMLRayExtension - Extension for generating VRML-Ray rendered images

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

#ifndef VRMLEXTENSION_H
#define VRMLEXTENSION_H

#include "vrmldialog.h"

#include <avogadro/glwidget.h>
#include <avogadro/extension.h>

class QProcess;

namespace Avogadro
{
	class VRMLExtension : public Extension
	{
		Q_OBJECT
			AVOGADRO_EXTENSION("VRML", tr("VRML"),
				tr("Create wrl files"))

	public:
		VRMLExtension(QObject* parent = 0);
		virtual ~VRMLExtension();

		/**
		* @return a list of actions which this widget can perform
		*/
		virtual QList<QAction *> actions() const;

		/**
		* @return the menu path for the specified action
		*/
		virtual QString menuPath(QAction* action) const;

		/**
		* @param action The action that triggered the calls.
		* @param widget The currently active GLWidget (feedback to the user).
		* @return An undo command for this action.
		*/
		virtual QUndoCommand* performAction(QAction *action, GLWidget *widget);



		public Q_SLOTS:

		/**
		* Slot to change the current molecule.
		*/
		void setMolecule(Molecule *molecule);

	private:
		GLWidget* m_glwidget;
		VRMLDialog* m_VRMLDialog;
		QList<QAction *> m_actions;
		Molecule *m_molecule;
		QProcess *m_process;

		private Q_SLOTS:
		void render();
		void calc();
		void finished(int exitCode);

	};

	class VRMLExtensionFactory : public QObject, public PluginFactory
	{
		Q_OBJECT
			Q_INTERFACES(Avogadro::PluginFactory)
			AVOGADRO_EXTENSION_FACTORY(VRMLExtension)
	};

} // End namespace Avogadro

#endif
