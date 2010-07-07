/**********************************************************************
  OrbitalExtension - Molecular orbital explorer

  Copyright (C) 2010 by David C. Lonie

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 ***********************************************************************/

#ifndef ORBITALWIDGET_H
#define ORBITALWIDGET_H

#include <QtGui/QWidget>

#include <avogadro/primitive.h>
#include <avogadro/glwidget.h>

#include "orbitalextension.h"
#include "orbitaltablemodel.h"

#include "ui_orbitalwidget.h"

namespace Avogadro {
  class OrbitalSettingsDialog;
  class OrbitalTableModel;

  class OrbitalWidget : public QWidget
  {
      Q_OBJECT

    public:

      enum OrbitalQuality {
        OQ_Low = 0,
        OQ_Medium,
        OQ_High,
        OQ_VeryHigh
      };

      //! Constructor
      explicit OrbitalWidget( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Deconstructor
      virtual ~OrbitalWidget();

      double isovalue() {return m_isovalue;};
      OrbitalQuality defaultQuality() {return m_quality;};

      static double OrbitalQualityToDouble(OrbitalQuality q);
      static double OrbitalQualityToDouble(int i) {
        return OrbitalQualityToDouble(OrbitalQuality(i));};

    public slots:
      void readSettings();
      void writeSettings();
      void reject();
      void fillTable(QList<Orbital> list);
      void setQuality(OrbitalQuality q);
      void selectOrbital(unsigned int orbital);
      void setDefaults(OrbitalWidget::OrbitalQuality quality, double isovalue, bool HOMOFirst);

    signals:
      void orbitalSelected(unsigned int orbital);
      void renderRequested(unsigned int orbital, double resolution);
      void calculateAll();

    private slots:
      void tableClicked(const QItemSelection&);
      void renderClicked();
      void configureClicked();

    private:
      Ui::OrbitalWidget ui;
      OrbitalSettingsDialog *m_settings;
      OrbitalQuality m_quality;
      double m_isovalue;

      OrbitalTableModel *m_tableModel;
      OrbitalSortingProxyModel *m_sortedTableModel;
  };
}

#endif
