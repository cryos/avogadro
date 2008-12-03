/**********************************************************************
  ShaderExtension - Extension for loading and using OpenGL 2.0 GLSL shaders

  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

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

#include "shaderextension.h"

#ifdef ENABLE_GLSL
  #include <GL/glew.h>
#endif

#include "../config.h"

#include <avogadro/glwidget.h>
#include <avogadro/toolgroup.h>

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

namespace Avogadro
{

  class Shader
  {
  public:
    Shader(QByteArray& vertSource, QByteArray& fragSource)
    {
      const char *cVert = vertSource.data();
      const char *cFrag = fragSource.data();
      vertexShader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertexShader, 1, &cVert, 0);
      glCompileShader(vertexShader);
      fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragmentShader, 1, &cFrag, 0);
      glCompileShader(fragmentShader);
      shaderProgram = glCreateProgram();
      glAttachShader(shaderProgram, vertexShader);
      glAttachShader(shaderProgram, fragmentShader);
      glLinkProgram(shaderProgram);
    }

    ~Shader()
    { 
      glDeleteProgram(shaderProgram);
      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);
    }

    GLuint shaderProgram, vertexShader, fragmentShader;
    QString name, description;
  };

  using Eigen::Vector3d;

  ShaderExtension::ShaderExtension(QObject* parent) : Extension(parent),
    m_glwidget(0), m_molecule(0), m_shaderDialog(0)
  {
    QAction* action = new QAction(this);
    action->setText(tr("GLSL Shaders..."));
    m_actions.append(action);
  }

  ShaderExtension::~ShaderExtension()
  {
    foreach(Shader *shader, m_shaders) {
      delete shader;
    }
    if (m_shaderDialog) {
      m_shaderDialog->deleteLater();
    }
  }

  QList<QAction *> ShaderExtension::actions() const
  {
    return m_actions;
  }

  QString ShaderExtension::menuPath(QAction*) const
  {
    return tr("&Extensions");
  }

  QUndoCommand* ShaderExtension::performAction(QAction *, GLWidget *widget)
  {
    m_glwidget = widget;
    if (!m_shaderDialog) {
      m_shaderDialog = new ShaderDialog();
      populateEngineCombo();
      loadShaders();
      populateShaderCombo();
      m_shaderDialog->show();

      connect(m_shaderDialog->shaderButton, SIGNAL(clicked()),
              this, SLOT(setShader()));
    }
    else {
      m_shaderDialog->show();
    }

    return 0;
  }

  void ShaderExtension::writeSettings(QSettings &settings) const
  {
    Extension::writeSettings(settings);
  }

  void ShaderExtension::readSettings(QSettings &settings)
  {
    Extension::readSettings(settings);
  }

  void ShaderExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  void ShaderExtension::setShader()
  {
    QString engineName = m_shaderDialog->displayTypes->currentText();
    GLuint shader = 0;
    // If the combo index is greater than zero we actually want a shader
    if (m_shaderDialog->shaderPrograms->currentIndex()) {
      shader = m_shaders[m_shaderDialog->shaderPrograms->currentIndex()-1]->shaderProgram;
    }
    foreach (Engine *engine, m_glwidget->engines()) {
      if (engine->name() == engineName) {
        engine->setShader(shader);
        m_glwidget->update();
        return;
      }
    }
  }

  void ShaderExtension::populateEngineCombo()
  {
    m_shaderDialog->displayTypes->clear();
    foreach (Engine *engine, m_glwidget->engines()) {
      m_shaderDialog->displayTypes->addItem(engine->name());
    }
  }

  void ShaderExtension::populateShaderCombo()
  {
    m_shaderDialog->shaderPrograms->clear();
    m_shaderDialog->shaderPrograms->addItem("None");
    foreach (Shader *shader, m_shaders) {
      m_shaderDialog->shaderPrograms->addItem(shader->name);
    }
  }

  void ShaderExtension::loadShaders()
  {
    // Now for the system wide shaders
    QDir verts, frags;
    QString systemShadersPath = QString(INSTALL_PREFIX) + '/'
      + "share/libavogadro/shaders";
    verts.cd(systemShadersPath);
    frags.cd(systemShadersPath);

    QStringList filters;
    filters << "*.vert";
    verts.setNameFilters(filters);
    verts.setFilter(QDir::Files | QDir::Readable);
    foreach(const QString& file, verts.entryList()) {
      qDebug() << file;
    }
    filters.clear();
    filters << "*.frag";
    frags.setNameFilters(filters);
    frags.setFilter(QDir::Files | QDir::Readable);
    foreach(const QString& file, frags.entryList()) {
      qDebug() << file;
    }

    if (frags.entryList().size() == verts.entryList().size()) {
      for (int i = 0; i < verts.entryList().size(); ++i) {
        QFile vertFile(verts.filePath(verts.entryList().at(i)));
        if (!vertFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
          qDebug() << "Error opening vert file:"
                   << QFileInfo(verts.entryList().at(i)).baseName();
          qDebug() << "Error:" << vertFile.error();
          continue;
        }
        QFile fragFile(frags.filePath(frags.entryList().at(i)));
        if (!fragFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
          qDebug() << "Error opening frag file...";
          continue;
        }
        QByteArray vertSource = vertFile.readAll();
        QByteArray fragSource = fragFile.readAll();
        vertFile.close();
        vertFile.close();
        qDebug() << "Shader loaded:" << QFileInfo(verts.entryList().at(i)).baseName();
//                 << "\n" << vertSource << "\n" << fragSource << "\n";
        Shader *shader = new Shader(vertSource, fragSource);
        shader->name = QFileInfo(verts.entryList().at(i)).baseName();
        m_shaders.push_back(shader);
      }
    }
  }

} // End namespace Avogadro

#include "shaderextension.moc"

Q_EXPORT_PLUGIN2(shaderextension, Avogadro::ShaderExtensionFactory)
