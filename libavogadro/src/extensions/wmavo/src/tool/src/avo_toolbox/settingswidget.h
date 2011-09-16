
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy

  This file is part of WmAvo (WiiChem project)
  WmAvo - Integrate the Wiimote and the Nunchuk in Avogadro software for the
  handling of the atoms and the camera.
  For more informations, see the README file.

  WmAvo is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  WmAvo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WmAvo. If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#pragma once
#ifndef __SETTINGSWIDGET_H__
#define __SETTINGSWIDGET_H__


#include "warning_disable_begin.h"
#include "variousfeatures.h"
#include "wmavo_const.h"
#include <QObject>
#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QSlider>
#include "warning_disable_end.h"


class SettingsWidget : public QObject
{
  Q_OBJECT

public : 
  SettingsWidget() ;
  ~SettingsWidget() ;

  inline QWidget* getSettingsWidget(){ return m_settingsWidget ; } ;
  inline QSlider* getSliderIRSensitive(){ return m_irSensitiveSlider ; } ;
  inline QSlider* getSliderPointSizeFont(){ return m_wmPointSizeFontSlider ; } ;
  inline QCheckBox* getCheckboxVibration(){ return m_checkBoxActivateVibration ; } ;
  inline QCheckBox* getCheckboxSleepThread(){ return m_checkBoxActivateSleepThread ; } ;
  inline QLabel* getLabelWmWorks(){ return m_labelWmWorks ; } ;

  void resetWidget() ;
  
public slots :

  inline void setWmWorksGood(){ if( m_labelWmWorks!=NULL ) m_labelWmWorks->setText(m_wmWorks) ; } ;
  inline void setWmWorksBad(){ if( m_labelWmWorks!=NULL ) m_labelWmWorks->setText(m_wmWorksBad) ; } ;

private :

  void createSettingsWidget() ;

private :
  QWidget *m_settingsWidget ; // (object)
  QSlider *m_irSensitiveSlider ; // (object)
  QSlider *m_wmPointSizeFontSlider ; // (object)
  QCheckBox *m_checkBoxActivateVibration ; // (object)
  QCheckBox *m_checkBoxActivateSleepThread ; // (object)
  QLabel *m_labelWmWorks ; // (object)
  QString m_wmWorks, m_wmWorksBad ;

} ;

#endif

