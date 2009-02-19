/**********************************************************************
  ShaderExtension - Extension for loading and using OpenGL 2.0 GLSL shaders

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#ifndef SHADEREXTENSION_H
#define SHADEREXTENSION_H

#include <avogadro/extension.h>

#include "ui_shaderdialog.h"

namespace Avogadro
{
  class GLWidget;
  class Shader;
  class ShaderDialog;

  class ShaderExtension : public Extension
  {
  Q_OBJECT
  AVOGADRO_EXTENSION("GLSL Shaders", tr("GLSL Shaders"))

  public:
    ShaderExtension(QObject* parent = 0);
    virtual ~ShaderExtension();

    virtual QString description() const
    {
      return QObject::tr("Load and use OpenGL 2.0 GLSL shaders");
    }

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

    /**
     * Save the settings for this extension.
     * @param settings Settings variable to write settings to.
     */
    virtual void writeSettings(QSettings &settings) const;

    /**
     * Read the settings for this extension.
     * @param settings Settings variable to read settings from.
     */
    virtual void readSettings(QSettings &settings);

  public Q_SLOTS:
    /**
     * Slot to change the current molecule.
     */
    void setMolecule(Molecule *molecule);

  private Q_SLOTS:
    /**
     * Slot to assign a shader program to an engine
     */
    void setShader();

    /**
     * Slot to load up a new vertex shader.
     */
    void loadShader();

    /**
     * Slot to choose the vertex file.
     */
    void loadVertFileDialog();

    /**
     * Slot to choose the vertex file.
     */
    void loadFragFileDialog();

    /**
     * Slot to choose the vertex file.
     */
    void loadParamsFileDialog();

  private:
    GLWidget *m_glwidget;
    QList<QAction *> m_actions;
    Molecule *m_molecule;
    QList<Shader *> m_shaders;
    ShaderDialog *m_shaderDialog;

    void populateEngineCombo();
    void populateShaderCombo();
    void loadShaders();

  };

  class ShaderDialog : public QDialog, public Ui::ShaderDialog
  {
    public:
      ShaderDialog(QDialog *parent=0) : QDialog(parent) {
        setupUi(this);
      }
  };

  class ShaderExtensionFactory : public QObject, public PluginFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::PluginFactory)
      AVOGADRO_EXTENSION_FACTORY(ShaderExtension, "GLSL Shader Extensions",
        tr("GLSL Shader Extension"),
        tr("Extension for loading and using GLSL shader programs."))
  };

} // End namespace Avogadro

#endif

