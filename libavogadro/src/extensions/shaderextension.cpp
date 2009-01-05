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
    Shader(QByteArray* vertSource, QByteArray* fragSource)
    {
      // Not all shaders need a fragment shader
      if (vertSource->isEmpty()) {
        qDebug() << "Empty vertex shader source sent to the shader constructor.";
        return;
      }
      if (fragSource) {
        if (fragSource->isEmpty()) {
          qDebug() << "Empty fragment shader source sent to the shader constructor.";
          return;
        }
      }
      shaderProgram = glCreateProgramObjectARB();
      const char *cVert = vertSource->data();
      vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
      glShaderSourceARB(vertexShader, 1, &cVert, 0);
      glCompileShaderARB(vertexShader);
      glAttachObjectARB(shaderProgram, vertexShader);

      if (fragSource) {
        const char *cFrag = fragSource->data();
        fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
        glShaderSourceARB(fragmentShader, 1, &cFrag, 0);
        glCompileShaderARB(fragmentShader);
        glAttachObjectARB(shaderProgram, fragmentShader);
      }
      glLinkProgramARB(shaderProgram);
    }

    ~Shader()
    {
      // First detach the shaders
      glDetachObjectARB(shaderProgram, vertexShader);
      glDetachObjectARB(shaderProgram, fragmentShader);
      // Now the shaders can be deleted
      glDeleteObjectARB(vertexShader);
      glDeleteObjectARB(fragmentShader);
      // Finally the program can be deleted
      glDeleteObjectARB(shaderProgram);
    }

    bool loadParameters(QByteArray* params)
    {
      // It appears you need to be using the shader to assign values to it
      if (params->isEmpty()) {
        qDebug() << "Empty parameters sent to the load parameters function.";
        return false;
      }
      glUseProgramObjectARB(shaderProgram);
      QList<QByteArray> lines = params->split('\n');
      qDebug() << "Loading shader parameter file:" << lines.size();
      qDebug() << *params;
      foreach(QByteArray line, lines) {
        QList<QByteArray> halves = line.split('\t');
        QList<QByteArray> tokens = halves.at(0).split(' ');
        if (tokens.size() != 2) {
          qDebug() << "Line not correctly space delimited:" << line;
          continue;
        }
        if (halves.size() != 2) {
          qDebug() << "Line not correctly tab delimited:" << line;
          continue;
        }
        // Retrieve the position of the variable
        const char *name = tokens.at(1).data();
        GLint pos = glGetUniformLocationARB(shaderProgram, name);
        if (pos < 0) {
          qDebug() << "Error, variable" << tokens.at(1) << "not found.";
          qDebug() << line;
          qDebug() << "Position:" << pos;
          continue;
        }
        if (tokens.at(0) == "float") {
          qDebug() << pos << "float line processed:" << line;
          glUniform1fARB(pos, halves.at(1).toFloat());
        }
        else if (tokens.at(0) == "vec3") {
          QList<QByteArray> numbers = halves.at(1).split(' ');
          if (numbers.size() != 3) {
            qDebug() << "Numbers not space delimited/wrong number, size:"
                     << numbers.size() << "token:" << halves.at(1);
            qDebug() << "Line:" << line;
          }
          else {
            qDebug() << pos << "vec3 line processed:" << line;
            glUniform3fARB(pos, numbers.at(0).toFloat(),
                                numbers.at(1).toFloat(),
                                numbers.at(2).toFloat());
          }
        }
        else if (tokens.at(0) == "vec4") {
          QList<QByteArray> numbers = halves.at(1).split(' ');
          if (numbers.size() != 4) {
            qDebug() << "Numbers not space delimited/wrong number, size:"
                     << numbers.size() << "token:" << halves.at(1);
            qDebug() << "Line:" << line;
          }
          else {
            qDebug() << pos << "vec4 line processed:" << line;
            glUniform4fARB(pos, numbers.at(0).toFloat(),
                                numbers.at(1).toFloat(),
                                numbers.at(2).toFloat(),
                                numbers.at(2).toFloat());
          }
        }
      }
      glUseProgramObjectARB(0);
      return true;
    }

    GLuint shaderProgram, vertexShader, fragmentShader;
    QString name, description;
  };

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

    bool glslEnabled = false;
    if (GLEW_VERSION_2_0) {
      glslEnabled = true;
    } else if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader) {
      glslEnabled = true;
    }
    if (!glslEnabled) {
      QMessageBox::warning(widget, "no GLSL support", "This system doesn't support shaders.");
      return 0;
    }

    if (!m_shaderDialog) {
      m_shaderDialog = new ShaderDialog();
      populateEngineCombo();
      loadShaders();
      populateShaderCombo();
      m_shaderDialog->show();

      connect(m_shaderDialog->shaderButton, SIGNAL(clicked()),
              this, SLOT(setShader()));
      connect(m_shaderDialog->vertFileButton, SIGNAL(clicked()),
              this, SLOT(loadVertFileDialog()));
      connect(m_shaderDialog->fragFileButton, SIGNAL(clicked()),
              this, SLOT(loadFragFileDialog()));
      connect(m_shaderDialog->paramsFileButton, SIGNAL(clicked()),
              this, SLOT(loadParamsFileDialog()));
      connect(m_shaderDialog->loadVertButton, SIGNAL(accepted()),
              this, SLOT(loadShader()));
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

  void ShaderExtension::loadShader()
  {
    if (!m_shaderDialog->vertFile->text().length()) {
      return;
    }
    QFileInfo vertInfo(m_shaderDialog->vertFile->text());
    if (vertInfo.exists()) {
      QFile vertFile(vertInfo.absoluteFilePath());
      if (!vertFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening vert file:" << vertInfo.absoluteFilePath();
        return;
      }
      Shader *shader = 0;
      QByteArray vertSource = vertFile.readAll();
      vertFile.close();
      // The shader file exists, check for a fragment file
      QFileInfo fragInfo(m_shaderDialog->fragFile->text());
      if (fragInfo.exists()) {
        QFile fragFile(fragInfo.absoluteFilePath());
        if (!fragFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
          qDebug() << "Error opening vert file:" << fragInfo.absoluteFilePath();
          return;
        }
        QByteArray fragSource = fragFile.readAll();
        fragFile.close();
        shader = new Shader(&vertSource, &fragSource);
      }
      else {
        shader = new Shader(&vertSource, 0);
      }
      qDebug() << "Shader loaded:" << vertInfo.baseName();

      shader->name = m_shaderDialog->shaderName->text();
      m_shaders.push_back(shader);

      // Now check for a parameter file and load it if necessary
      if (m_shaderDialog->paramsFile->text().size()) {
        QFileInfo paramsInfo(m_shaderDialog->paramsFile->text());
        if (paramsInfo.exists()) {
          QFile paramsFile(paramsInfo.absoluteFilePath());
          if (!paramsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Error opening parameters file..."
                    << paramsInfo.absoluteFilePath();
            return;
          }
          QByteArray params = paramsFile.readAll();
          paramsFile.close();
          if (!shader->loadParameters(&params)) {
            qDebug() << "Error reading parameter file in."
                     << paramsInfo.absoluteFilePath();
          }
        }
      }
      populateShaderCombo();
    }
  }

  void ShaderExtension::loadVertFileDialog()
  {
    // Load a file
    QString fileName = QFileDialog::getOpenFileName(m_shaderDialog,
      tr("Open a vertex shader source file"), m_shaderDialog->vertFile->text(),
      tr("Vertex shader files (*.vert)"));
    if (!fileName.isEmpty()) {
      m_shaderDialog->vertFile->setText(fileName);
      QFileInfo vert(fileName);
      // See if we can guess the other two from this
      vert.setFile(vert.absolutePath() + "/" + vert.baseName() + ".frag");
      if (vert.exists()) {
        m_shaderDialog->fragFile->setText(vert.absoluteFilePath());
      }
      vert.setFile(vert.absolutePath() + "/" + vert.baseName() + ".params");
      if (vert.exists()) {
        m_shaderDialog->paramsFile->setText(vert.absoluteFilePath());
      }
    }
  }

  void ShaderExtension::loadFragFileDialog()
  {
    // Load a file
    QString fileName = QFileDialog::getOpenFileName(m_shaderDialog,
      tr("Open a fragment shader source file"), m_shaderDialog->fragFile->text(),
      tr("Fragment shader files (*.frag)"));
    if (!fileName.isEmpty()) {
      m_shaderDialog->fragFile->setText(fileName);
    }
  }

  void ShaderExtension::loadParamsFileDialog()
  {
    // Load a file
    QString fileName = QFileDialog::getOpenFileName(m_shaderDialog,
      tr("Open a shader parameters file"), m_shaderDialog->paramsFile->text(),
      tr("Shader parameters files (*.params)"));
    if (!fileName.isEmpty()) {
      m_shaderDialog->paramsFile->setText(fileName);
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
    QDir verts;
    #ifdef WIN32
    verts = QCoreApplication::applicationDirPath();
    verts.cd("shaders");
    #else
    QString systemShadersPath = QString(INSTALL_PREFIX) + '/'
      + "share/libavogadro/shaders";
    verts.cd(systemShadersPath);
    #endif

    QStringList filters;
    filters << "*.vert";
    verts.setNameFilters(filters);
    verts.setFilter(QDir::Files | QDir::Readable);

    for (int i = 0; i < verts.entryList().size(); ++i) {
      Shader *shader = 0;
      QFileInfo info(verts.filePath(verts.entryList().at(i)));
      QFile vertFile(info.absoluteFilePath());
      if (!vertFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening vert file:" << info.absoluteFilePath();
        continue;
      }
      QByteArray vertSource = vertFile.readAll();
      vertFile.close();
      // Is there a corresponding fragment file?
      if (verts.exists(info.baseName() + ".frag")) {
        QFile fragFile(info.canonicalPath() + "/" + info.baseName() + ".frag");
        if (!fragFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
          qDebug() << "Error opening frag file..."
                   << info.canonicalPath() + "/" + info.baseName() + ".frag";
          continue;
        }
        QByteArray fragSource = fragFile.readAll();
        vertFile.close();
        shader = new Shader(&vertSource, &fragSource);
      }
      else {
        shader = new Shader(&vertSource, 0);
      }
      qDebug() << "Shader loaded:" << info.baseName();

      shader->name = info.baseName();
      m_shaders.push_back(shader);

      // Now let us see if there are any parameter files that need loading...
      if (verts.exists(info.baseName() + ".params")) {
        QFile paramsFile(info.canonicalPath() + "/" + info.baseName() + ".params");
        if (!paramsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
          qDebug() << "Error opening parameters file..."
                   << info.canonicalPath() + "/" + info.baseName() + ".params";
          continue;
        }
        QByteArray params = paramsFile.readAll();
        paramsFile.close();
        if (!shader->loadParameters(&params)) {
          qDebug() << "Error reading parameter file in." << info.baseName();
        }
      }
    }
  }

} // End namespace Avogadro

#include "shaderextension.moc"

Q_EXPORT_PLUGIN2(shaderextension, Avogadro::ShaderExtensionFactory)
