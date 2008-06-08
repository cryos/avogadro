/**********************************************************************
  Overlay - 2D Overlay Extension

  Copyright (C) 2008 by Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Some code is based on Open Babel
  For more information, see <http://openbabel.sourceforge.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#include "overlayextension.h"

#include <QAction>
#include <QColor>
#include <QGraphicsTextItem>

using namespace std;
using namespace OpenBabel;

namespace OpenBabel{
  extern OBElementTable etab;
}

namespace Avogadro
{

  // this is a trick to identify what action we are taking
  enum OverlayExtensionIndex {
    ESPAction = 0,
    MoleculeAction
  };

  OverlayExtension::OverlayExtension( QObject *parent ) : Extension( parent ), m_espView(0), m_moleculeView(0)
  {
    QAction *action = new QAction( this );
    action->setText( tr("ESP Overlay" ));
    m_actions.append( action );
    action->setData( ESPAction );

    action = new QAction( this );
    action->setText( tr("Molecule Overlay" ));
    m_actions.append( action );
    action->setData( MoleculeAction );
 
  }

  OverlayExtension::~OverlayExtension()
  {
    //if (m_espView)
    //  delete m_espView;
  }

  QList<QAction *> OverlayExtension::actions() const
  {
    return m_actions;
  }

  // allows us to set the intended menu path for each action
  QString OverlayExtension::menuPath(QAction *action) const
  {
    int i = action->data().toInt();

    switch ( i ) {
      case ESPAction:
      case MoleculeAction:
        return tr("&Extensions") + '>' + tr("&Overlays");
        break;
    }
    return "";
  }
  
  QUndoCommand* OverlayExtension::performAction(QAction *action, GLWidget *widget)
  {
    int i = action->data().toInt();

    switch ( i ) {
      case ESPAction:
	if (!m_espView)
	  espOverlay(widget);
        break;
      case MoleculeAction:
	if (!m_moleculeView)
	  moleculeOverlay(widget->molecule(), widget);
        break;
    }

    return 0;
  }
 
  void OverlayExtension::moleculeOverlay(Molecule *molecule, GLWidget *widget)
  {
    QGraphicsScene *scene = new QGraphicsScene();
    scene->setBackgroundBrush(QColor(0, 0, 0));

    QFont font("sans-serif");
    font.setPixelSize(18);
    QFontMetrics fm(font);
    int pixelHeight = fm.height();

    molecule->Center();

    FOR_BONDS_OF_MOL (bond, molecule) {
      QPen pen(QColor(0,255,0));
      if (bond->IsHash())
        pen.setStyle(Qt::DashLine);
      if (bond->IsWedge())
        pen.setWidth(3);
      QGraphicsLineItem *line = scene->addLine(QLineF(bond->GetBeginAtom()->GetX(), bond->GetBeginAtom()->GetY(),
                            bond->GetEndAtom()->GetX(), bond->GetEndAtom()->GetY()), pen);
      line->setZValue(1);
    }

    FOR_ATOMS_OF_MOL (atom, molecule) {
      if (!atom->IsCarbon()) {
        QString symbol = OpenBabel::etab.GetSymbol(atom->GetAtomicNum());
        //QGraphicsTextItem *text = scene->addText(symbol, font);
        QGraphicsTextItem *text = new QGraphicsTextItem(symbol);
	switch (atom->GetAtomicNum()) {
	  case 1:
            text->setDefaultTextColor(QColor(70, 70, 70));
	    break;
	  case 7:
            text->setDefaultTextColor(QColor(0, 0, 255));
	    break;
          case 8:
            text->setDefaultTextColor(QColor(255, 0, 0));
	    break;
	}
        QRectF rect = text->boundingRect();
        rect.setRect(atom->GetX() - rect.width() / 2, atom->GetY() - pixelHeight / 2,
                     rect.width(), rect.height());
	QGraphicsRectItem *rectItem = scene->addRect(rect, QColor(0,0,0));
	rectItem->setBrush(QColor(0,0,0));
        rectItem->setZValue(2);
	text->setZValue(3);
	scene->addItem(text);
        text->setPos(QPointF(atom->GetX() - rect.width() / 2, atom->GetY() - pixelHeight / 2));
      }
    }
	
    m_moleculeView = new QGraphicsView(widget);
    m_moleculeView->setRenderHint(QPainter::Antialiasing);
    connect(widget, SIGNAL(resized()), this, SLOT(moleculeResize()));
    //m_view->setGeometry((widget->width() - 600) / 2, (widget->height() - 500) / 2, 600, 500);
    m_moleculeView->setGeometry(0, widget->height() / 2, widget->width() / 2, widget->height() / 2);
    m_moleculeView->setScene(scene);
    m_moleculeView->setFrameShape(QFrame::NoFrame);
    m_moleculeView->show();
  }

 
  void OverlayExtension::espOverlay(GLWidget *widget)
  {
    QGraphicsScene *scene = new QGraphicsScene();
    scene->setBackgroundBrush(QColor(0, 0, 0));

    QLinearGradient gradient(QPointF(0, 0), QPointF(300, 0));
    gradient.setColorAt(0, Qt::red);
    gradient.setColorAt(0.5, Qt::green);
    gradient.setColorAt(1, Qt::blue);
    QGraphicsRectItem *rectangle = scene->addRect(QRectF(0, 0, 300, 45), QPen(), gradient);

    QFont font("sans-serif");
    font.setPixelSize(18);
    QFontMetrics fm(font);
    int pixelHeight = fm.height();

    QGraphicsTextItem *text1 = scene->addText("-1.0", font);
    text1->setDefaultTextColor(QColor(255,255,255));
    text1->setPos(QPointF(-45, (45 - pixelHeight) / 2 ));
	
    QGraphicsTextItem *text2 = scene->addText("+1.0", font);
    text2->setDefaultTextColor(QColor(255,255,255));
    text2->setPos(QPointF(305, (45 - pixelHeight) / 2 ));
        
    m_espView = new QGraphicsView(widget);
    connect(widget, SIGNAL(resized()), this, SLOT(espResize()));
    m_espView->setGeometry(widget->width() / 2 - 200, widget->height() - 60, 420, 50);
    m_espView->setScene(scene);
    m_espView->setFrameShape(QFrame::NoFrame);
    m_espView->show();
  }

  void OverlayExtension::espResize()
  {
    GLWidget *widget = (GLWidget*) m_espView->parent();
    m_espView->setGeometry(widget->width() / 2 - 200, widget->height() - 60, 400, 50);
  }

  void OverlayExtension::moleculeResize()
  {
    GLWidget *widget = (GLWidget*) m_moleculeView->parent();
    m_moleculeView->setGeometry(0, widget->height() / 2, widget->width() / 2, widget->height() / 2);
  }


}

#include "overlayextension.moc"

Q_EXPORT_PLUGIN2(overlayextension, Avogadro::OverlayExtensionFactory)
