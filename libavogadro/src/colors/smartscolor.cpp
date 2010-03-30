/**********************************************************************
  SmartsColor -  Map atom colors based on atom partial charge

  Copyright (C) 2010 Geoffrey R. Hutchison

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

#include "smartscolor.h"

#include <avogadro/primitive.h>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/colorbutton.h>

#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/parsmart.h>

#include <QtPlugin>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>

using namespace OpenBabel;

namespace Avogadro {

  /// Constructor
  SmartsColor::SmartsColor() : _highlightColor(255, 0, 128), _settingsWidget(NULL)
  { 
    _pattern = new OBSmartsPattern;
    _smartsString.clear();
  }

  /// Destructor
  SmartsColor::~SmartsColor()
  { 
    if (_pattern) {
      delete _pattern;
      _pattern = NULL;
    }

    if (_settingsWidget)
      _settingsWidget->deleteLater();
  }

  void SmartsColor::settingsWidgetDestroyed()
  {
    _settingsWidget = 0;
  }

  QWidget *SmartsColor::settingsWidget()
  {
    if (!_settingsWidget) {
      _settingsWidget = new QWidget();
      QHBoxLayout *layout = new QHBoxLayout(_settingsWidget);
      QVBoxLayout *vlayout1 = new QVBoxLayout(_settingsWidget);
      QVBoxLayout *vlayout2 = new QVBoxLayout(_settingsWidget);
      QLabel *label1 = new QLabel(tr("SMARTS Pattern:"), _settingsWidget);
      vlayout1->addWidget(label1);
      QLineEdit *smartsLine = new QLineEdit(_smartsString, _settingsWidget);
      vlayout2->addWidget(smartsLine);
      QLabel *label2 = new QLabel(tr("Highlight Color:"), _settingsWidget);
      ColorButton *button = new ColorButton(_highlightColor, _settingsWidget);
      vlayout1->addWidget(label2);
      vlayout2->addWidget(button);
      layout->addLayout(vlayout1);
      layout->addLayout(vlayout2);

      connect(button, SIGNAL(colorChanged(QColor)),
              this, SLOT(colorChanged(QColor)));
      connect(smartsLine, SIGNAL(textChanged(QString)),
              this, SLOT(smartsChanged(QString)));
      connect(_settingsWidget, SIGNAL(destroyed()),
              this, SLOT(settingsWidgetDestroyed()));
    }

    return _settingsWidget;
  }

  void SmartsColor::colorChanged(QColor newColor)
  {
    _highlightColor = newColor;
    emit changed();
  }

  void SmartsColor::smartsChanged(QString newPattern)
  {
    _smartsString = newPattern;
    _pattern->Init(_smartsString.toAscii());
    emit changed();
  }

  void SmartsColor::writeSettings(QSettings &settings) const
  {
    settings.setValue("highlightcolor", _highlightColor);
    settings.setValue("SMARTS", _smartsString);
  }

  void SmartsColor::readSettings(QSettings &settings)
  {
    _highlightColor = settings.value("highlightcolor", QColor(Qt::magenta)).value<QColor>();
    _smartsString = settings.value("SMARTS").toString();
    smartsChanged(_smartsString);
  }

  void SmartsColor::setFromPrimitive(const Primitive *p)
  {
    if (!p || p->type() != Primitive::AtomType)
      return;

    const Atom *atom = static_cast<const Atom*>(p);

    // Start with the default "element color"
    QColor newcolor;
    if (atom->atomicNumber()) {
      std::vector<double> rgb = OpenBabel::etab.GetRGB(atom->atomicNumber());
      newcolor.setRgbF(rgb[0], rgb[1], rgb[2]);
    } else {
      newcolor.setRgbF(0.2f, 0.2f, 0.2f);
    }

    Molecule *molecule = qobject_cast<Molecule *>(atom->parent());
    if (!molecule || !_pattern)
      return;

    bool matched = false;
    if (!_smartsString.isEmpty() && _pattern->IsValid()) { // finite, valid SMARTS, so let's go for it!
      OBMol obmol = molecule->OBMol();
      bool moleculeMatched = _pattern->Match(obmol);

      if (moleculeMatched) {
        std::vector<std::vector<int> > mlist = _pattern->GetUMapList();
        std::vector<std::vector<int> >::iterator match;
        for (match = mlist.begin(); match != mlist.end(); ++match) { // iterate through matches
          for (unsigned idx = 0; idx < (*match).size(); ++idx) { // iterate through atoms in match
            if (atom->index() == ((*match)[idx] - 1)) { // TODO: OB uses index from 1
              matched = true;
              break;
            }
          } // atoms in match
          if (matched)
            break; // no need to check other matches
        } // matches
        
      } // matched molecule
    } // finite, valid SMARTS

    // OK, now highlight the SMARTS match
    if (matched)
      setFromQColor(_highlightColor);
    else
      setFromQColor(newcolor.darker());
    m_channels[3] = 1.0;
  }

}

Q_EXPORT_PLUGIN2(smartscolor, Avogadro::SmartsColorFactory)

