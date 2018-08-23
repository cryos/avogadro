/**********************************************************************
  OrbitalSettingsDialog -- Configure the Orbital Extension

  Copyright (C) 2010 David C. Lonie

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

#ifndef ORBITALSETTINGSDIALOG_H
#define ORBITALSETTINGSDIALOG_H

#include <QDialog>

#include "ui_orbitalsettingsdialog.h"
#include "orbitalwidget.h"

namespace Avogadro
{

  class OrbitalSettingsDialog : public QDialog
  {
    Q_OBJECT

  public:
    explicit OrbitalSettingsDialog(OrbitalWidget* parent, Qt::WindowFlags f = 0);
    ~OrbitalSettingsDialog();

  public slots:
    void setDefaultQuality(OrbitalWidget::OrbitalQuality);
    void setIsoValue(double);
    void setBoxPadding(double);
    void setHOMOFirst(bool);
    void setLimitPrecalc(bool);
    void setPrecalcRange(int);
    void updateDefaults();
    void updatePrecalcSettings();
    void accept();
    void reject();

  signals:
    void calculateAll();
    void defaultsUpdated(OrbitalWidget::OrbitalQuality quality, double isoval,
                         double boxPadding, bool HOMOFirst);
    void precalcSettingsUpdated(bool limit, int range);

  private slots:
    void calculateAllClicked();

  private:
    Ui::OrbitalSettingsDialog ui;
    OrbitalWidget::OrbitalQuality m_quality;
    double m_isoval;
    double m_boxPadding;
    bool m_HOMOFirst;
    bool m_limit_precalc;
    int m_precalc_range;
  };

} // End namespace Avogadro

#endif
