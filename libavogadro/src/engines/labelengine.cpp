/**********************************************************************
  LabelEngine - Engine for displaying labels.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007 Benoit Jacob
  Copyright (C) 2007,2008 Marcus D. Hanwell
  Some portions Copyright (C) 2009 Konstantin L. Tokarev

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

#include "labelengine.h"

#include <avogadro/elementtranslator.h>
#include <avogadro/camera.h>
#include <avogadro/painter.h>
#include <avogadro/painterdevice.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>
#include <avogadro/residue.h>
#include <avogadro/molecule.h>

#include <QDebug>
#include <QtGui/QColorDialog>
#include <QtGui/QFontDialog>
//#include <QtGui/QPainter>

#include <openbabel/mol.h>

using namespace std;
using namespace Eigen;


namespace Avogadro {

/*class DummyAtom : public Atom
{
    public:
    DummyAtom() {
        setIndex(0);
        setGroupIndex(1);
	    setAtomicNumber(12);
	    setPartialCharge(0.1);
	    setFormalCharge(0);
        setResidue(1);
    }
};

DummyAtom *dummyAtom;
Bond *dummyBond;*/

  LabelSettingsWidget::LabelSettingsWidget(QWidget *parent) : QWidget(parent) {
        setupUi(this);
	    }
  

 
  LabelEngine::LabelEngine(QObject *parent) : Engine(parent),
                    m_atomType(1), m_bondType(0),
                    m_atomColor(255,255,255), m_bondColor(255,255,255),
					m_settingsWidget(0),
                    m_displacement(0,0,0),  m_bondDisplacement(0,0,0)
  {
    /*dummyAtom.setGroupIndex(1);
	dummyAtom.setAtomicNumber(12);
	dummyAtom.setPartialCharge(0.1);
	dummyAtom.setFormalCharge(0);*/
	//dummyAtom.set
  }

  Engine *LabelEngine::clone() const
  {
    LabelEngine *engine = new LabelEngine(parent());
    engine->setAlias(alias());
    engine->setAtomType(m_atomType);
    engine->setBondType(m_bondType);
    engine->setEnabled(isEnabled());

    return engine;
  }

  bool LabelEngine::renderOpaque(PainterDevice *pd)
  {
    if (m_atomType > 0) {
      // Render atom labels
      foreach(Atom *a, atoms())
        renderOpaque(pd, a);
    }

    if (m_bondType > 0) {
      // Now render the bond labels
      foreach(Bond *b, bonds())
        renderOpaque(pd, b);
    }
	  
    return true;
  }

  bool LabelEngine::renderQuick(PainterDevice *)
  {
    // Don't render text when moving...
    return true;
  }

  bool LabelEngine::renderOpaque(PainterDevice *pd, const Atom *a)
  {
    // Render atom labels
    const Vector3d pos = *a->pos();

    double renderRadius = pd->radius(a);
    renderRadius += 0.05;

    double zDistance = pd->camera()->distance(pos);

    if(zDistance < 50.0) {
      QString str = createAtomLabel(a);

      Vector3d zAxis = pd->camera()->backTransformedZAxis();

      Vector3d drawPos = pos + zAxis * renderRadius + m_displacement;

      glColor3f(m_atomColor.redF(), m_atomColor.greenF(), m_atomColor.blueF());
      //pd->painter()->setPen(m_atomColor);
      //pd->painter()->setFont(m_atomFont);
      pd->painter()->drawText(drawPos, str); //, m_atomFont, m_atomColor);
    }

    return true;
  }

  QString LabelEngine::createAtomLabel(const Atom *a)
  {
	QString str;
    unsigned int gi;
    switch(m_atomType) {
      case 1: // Atom index
        str = QString("%L1").arg(a->index() + 1);
        break;
      case 2: // Element Symbol
        str = QString(OpenBabel::etab.GetSymbol(a->atomicNumber()));
        break;
      case 3: // Symbol & Number in Group
        gi = a->groupIndex();
        if (gi != 0) {
          str = QString(OpenBabel::etab.GetSymbol(a->atomicNumber())) + QString("%L1").arg(gi);
        } else {
          str = QString(OpenBabel::etab.GetSymbol(a->atomicNumber()));
        }
		break;
      case 4: // Symbol & Atom Number
        str = QString(OpenBabel::etab.GetSymbol(a->atomicNumber())) + QString("%L1").arg(a->index() + 1);
        break;
      case 5: // Formal charge
        if (a->formalCharge())
          str = QString("%L1").arg(a->formalCharge());
        break;
      case 6: // Partial charge
        str = QString("%L1").arg(const_cast<Atom *>(a)->partialCharge(), 0, 'g', 2);
        break;
      case 7: // Element name
        str = ElementTranslator::name(a->atomicNumber());
        break;
      case 8: // Residue name
        if (a->residue())
          str = a->residue()->name();
        break;
      case 9: // Residue number
        if (a->residue())
          str = a->residue()->number();
        break;
      case 10: // Unique ID
        str = QString("%L1").arg(a->id());
        break;
      default: // some custom data -- if available
        int customIndex = m_atomType - 7 - 1;
        QList<QByteArray> propertyNames = a->dynamicPropertyNames();
        // If this is a strange offset, use the element symbol
        if ( customIndex < 0 || customIndex >= propertyNames.size()) {
          str = QString(OpenBabel::etab.GetSymbol(a->atomicNumber()));
        }
        else
          str = a->property(propertyNames[customIndex].data()).toString();
    }
	return str;
  }

  QString LabelEngine::createBondLabel(const Bond *b)
  {
      QString str;
      switch(m_bondType) {
        str = "";
      case 1:
        str = QString("%L1").arg(b->length(), 0, 'g', 4);
        break;
      case 2:
        str = QString("%L1").arg(b->index() + 1);
        break;
      case 4:
        str = QString("%L1").arg(b->id());
        break;
      case 3:
      default:
        str = QString("%L1").arg(b->order());
      }
	  return str;
  }
  
  bool LabelEngine::renderOpaque(PainterDevice *pd, const Bond *b)
  {
    // Render bond labels
    Atom* atom1 = pd->molecule()->atomById(b->beginAtomId());
    Atom* atom2 = pd->molecule()->atomById(b->endAtomId());
    Vector3d v1 (*atom1->pos());
    Vector3d v2 (*atom2->pos());
    Vector3d d = v2 - v1;
    d.normalize();

    // Work out the radii of the atoms and the bond
    double renderRadius = pd->radius(b);
    double renderRadiusA1 = pd->radius(atom1);
    double renderRadiusA2 = pd->radius(atom2);
    // If the render radius is zero then this view does not draw bonds
    if (renderRadius < 1.0e-3)
      return false;

    renderRadius += 0.05;

    // Calculate the
    Vector3d pos ( (v1 + v2 + d*(renderRadiusA1-renderRadiusA2)) / 2.0 );

    double zDistance = pd->camera()->distance(pos);

    if(zDistance < 50.0) {
	  QString str = createBondLabel(b);

      Vector3d zAxis = pd->camera()->backTransformedZAxis();
      Vector3d drawPos = pos + zAxis * renderRadius + m_bondDisplacement;

      //glColor3f(1.0, 1.0, 1.0);
      glColor3f(m_bondColor.redF(), m_bondColor.greenF(), m_bondColor.blueF());
      //pd->painter()->setColor(m_bondColor);
      //pd->painter()->setFont(m_bondFont);
      pd->painter()->drawText(drawPos, str);//, m_bondFont, m_bondColor);
    }

    return true;
  }

  void LabelEngine::setAtomType(int value)
  {
    m_atomType = value;
    //QBrush brush(qRgb(0, 128, 0));
    //QPainter p(m_settingsWidget->atomLabel);
    //p.fillRect(m_settingsWidget->atomLabel->frameRect(), brush);
    //dummyAtom = new DummyAtom;
    //m_settingsWidget->atomLabel->setText(createAtomLabel(dummyAtom));
    //delete dummyAtom;
   //p.end();
    emit changed();
  }

  void LabelEngine::setBondType(int value)
  {
    m_bondType = value;
    //this->atomLabelColor->
    //dummyBond = new Bond;
    //m_settingsWidget->bondLabel->setText(createBondLabel(0));
    //delete dummyBond;
    emit changed();
  }

  void LabelEngine::updateDisplacement(double)
  {
      m_displacement = Vector3d(m_settingsWidget->xDisplSpinBox->value(),
                                m_settingsWidget->yDisplSpinBox->value(),
                                m_settingsWidget->zDisplSpinBox->value());
      emit changed();
  }

  void LabelEngine::updateBondDisplacement(double)
  {
      m_bondDisplacement = Vector3d(m_settingsWidget->xBondDisplSpinBox->value(),
                                m_settingsWidget->yBondDisplSpinBox->value(),
                                m_settingsWidget->zBondDisplSpinBox->value());
      emit changed();
  }
  
  QWidget *LabelEngine::settingsWidget()
  {
    if(!m_settingsWidget)
      {
        m_settingsWidget = new LabelSettingsWidget();
        m_settingsWidget->atomType->setCurrentIndex(m_atomType);
        m_settingsWidget->bondType->setCurrentIndex(m_bondType);
        setAtomType(m_atomType);
        setBondType(m_bondType);
        connect(m_settingsWidget->atomType, SIGNAL(activated(int)), this, SLOT(setAtomType(int)));
        connect(m_settingsWidget->atomColor, SIGNAL(clicked()), this, SLOT(setAtomColor()));
        connect(m_settingsWidget->atomFont, SIGNAL(clicked()), this, SLOT(setAtomFont()));
        connect(m_settingsWidget->bondType, SIGNAL(activated(int)), this, SLOT(setBondType(int)));
        connect(m_settingsWidget->bondColor, SIGNAL(clicked()), this, SLOT(setBondColor()));
        connect(m_settingsWidget->bondFont, SIGNAL(clicked()), this, SLOT(setBondFont()));        
        connect(m_settingsWidget, SIGNAL(destroyed()), this, SLOT(settingsWidgetDestroyed()));
        connect(m_settingsWidget->xDisplSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateDisplacement(double)));
        connect(m_settingsWidget->yDisplSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateDisplacement(double)));
        connect(m_settingsWidget->zDisplSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateDisplacement(double)));
        connect(m_settingsWidget->xBondDisplSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateBondDisplacement(double)));
        connect(m_settingsWidget->yBondDisplSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateBondDisplacement(double)));
        connect(m_settingsWidget->zBondDisplSpinBox, SIGNAL(valueChanged(double)),
              this, SLOT(updateBondDisplacement(double)));
      }
    return m_settingsWidget;
  }

  void LabelEngine::setAtomColor()
  {
    QColor current(m_atomColor);
    QColor color = QColorDialog::getColor(current, m_settingsWidget, tr("Select Atom Labels Color"));
    if (color.isValid() && color != current) {
      m_atomColor = color;
      emit changed();
    }
  }

  void LabelEngine::setAtomFont()
  {
    bool ok;
    QFont current(m_atomFont);
    QFont font = QFontDialog::getFont(&ok, current, m_settingsWidget, tr("Select Atom Labels Font"));
    if (ok) {
      m_atomFont = font;
      m_settingsWidget->atomLabel->setFont(m_atomFont);
      emit changed();
    }
  }

  void LabelEngine::setBondColor()
  {
    QColor current(m_bondColor);
    QColor color = QColorDialog::getColor(current, m_settingsWidget, tr("Select Bond Labels Color"));
    if (color.isValid() && color != current) {
      m_bondColor = color;
      emit changed();
    }
  }

  void LabelEngine::setBondFont()
  {
    bool ok;
    QFont current(m_bondFont);
    QFont font = QFontDialog::getFont(&ok, current, m_settingsWidget, tr("Select Bond Labels Font"));
    if (ok) {
      m_bondFont = font;
      m_settingsWidget->bondLabel->setFont(m_bondFont);
      emit changed();
    }
  }
  
  void LabelEngine::settingsWidgetDestroyed()
  {
    qDebug() << "Destroyed Settings Widget";
    m_settingsWidget = 0;
  }

  Engine::Layers LabelEngine::layers() const
  {
    return Engine::Overlay;
  }

  Engine::ColorTypes LabelEngine::colorTypes() const
  {
    return Engine::NoColors;
  }

  void LabelEngine::writeSettings(QSettings &settings) const
  {
    Engine::writeSettings(settings);
    settings.setValue("atomLabel", m_atomType);
    settings.setValue("bondLabel", m_bondType);
  }

  void LabelEngine::readSettings(QSettings &settings)
  {
    Engine::readSettings(settings);
    setAtomType(settings.value("atomLabel", 1).toInt());
    setBondType(settings.value("bondLabel", 0).toInt());
    if(m_settingsWidget) {
      m_settingsWidget->atomType->setCurrentIndex(m_atomType);
      m_settingsWidget->bondType->setCurrentIndex(m_bondType);
    }

  }
}

Q_EXPORT_PLUGIN2(labelengine, Avogadro::LabelEngineFactory)
