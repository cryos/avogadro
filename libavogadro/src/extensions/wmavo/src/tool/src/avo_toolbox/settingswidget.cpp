
/*******************************************************************************
  Copyright (C) 2010,2011 Mickael Gadroy, University of Reims Champagne-Ardenne (Fr)
                          Project managers: Eric Henon and Michael Krajecki
                          Financial support: Region Champagne-Ardenne (Fr)

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

#include "settingswidget.h"


SettingsWidget::SettingsWidget() 
  : m_settingsWidget(NULL), 
    m_irSensitiveSlider(NULL), m_wmPointSizeFontSlider(NULL),
    m_checkBoxActivateVibration(NULL), m_checkBoxActivateSleepThread(NULL),
    m_labelWmWorks(NULL)
{
  m_wmWorks = "Wiimote works good." ;
  m_wmWorksBad = "Wiimote can work better." ;
  createSettingsWidget() ;
}


SettingsWidget::~SettingsWidget()
{
  if( m_settingsWidget != NULL )
  {
    // All feature are deleted by the object itself.
    delete m_settingsWidget ;
  }
}


/**
  * Create the settings widget.
  */
void SettingsWidget::createSettingsWidget()
{
  if( m_settingsWidget == NULL )
  {
    m_settingsWidget = new QWidget ;

    QLabel *lblirSensitive = new QLabel(tr("Wiimote sensitive :")) ;
    QLabel *lblWmSMoins = new QLabel(tr("(-)")) ;
    QLabel *lblWmSPlus = new QLabel(tr("(+)")) ;
    m_irSensitiveSlider = new QSlider( Qt::Horizontal ) ;
    m_irSensitiveSlider->setMaximum( PLUGIN_WM_SENSITIVE_MAX ) ;
    m_irSensitiveSlider->setMinimum( PLUGIN_WM_SENSITIVE_MIN ) ;
    m_irSensitiveSlider->setValue( PLUGIN_WM_SENSITIVE_DEFAULT ) ;
    m_irSensitiveSlider->setTickInterval( 1 ) ;
    //m_addHydrogensCheck = new QCheckBox( "Adjust Hydrogen" ) ;

    QLabel *lblWmPointSize = new QLabel(tr("Font Size :")) ;
    QLabel *lblWmMoins = new QLabel(tr("(-)")) ;
    QLabel *lblWmPlus = new QLabel(tr("(+)")) ;
    m_wmPointSizeFontSlider = new QSlider( Qt::Horizontal ) ;
    m_wmPointSizeFontSlider->setMaximum( (int)(WMTOOL_POINTSIZE_RATIO_MAX * 10.0f) ) ;
    m_wmPointSizeFontSlider->setMinimum( (int)(WMTOOL_POINTSIZE_RATIO_MIN * 10.0f) ) ;
    m_wmPointSizeFontSlider->setValue( (int)(WMTOOL_POINTSIZE_RATIO_DEFAULT * 10.0f) ) ;
    m_wmPointSizeFontSlider->setTickInterval( 1 ) ;

    QLabel *lblVibration = new QLabel(tr("Vibration :")) ;
    m_checkBoxActivateVibration = new QCheckBox( "ON/OFF" ) ;
    m_checkBoxActivateVibration->setChecked(PLUGIN_WM_VIBRATION_ONOFF) ;

    QLabel *lblSleepThread = new QLabel(tr("Less CPU used (but can lag):")) ;
    m_checkBoxActivateSleepThread = new QCheckBox( "ON/OFF" ) ;
    m_checkBoxActivateSleepThread->setChecked(PLUGIN_WM_SLEEPTHREAD_ONOFF) ;
    
    m_labelWmWorks = new QLabel( m_wmWorks ) ;

    QVBoxLayout *vBoxSens=new QVBoxLayout() ;
    QHBoxLayout *hBoxSens=new QHBoxLayout() ;
    hBoxSens->addWidget( lblWmSMoins ) ;
    hBoxSens->addWidget( m_irSensitiveSlider ) ;
    hBoxSens->addWidget( lblWmSPlus ) ;
    vBoxSens->addWidget( lblirSensitive ) ;
    vBoxSens->addLayout( hBoxSens ) ;
    vBoxSens->addStretch( 1 ) ;

    QVBoxLayout *vBoxPointSize=new QVBoxLayout() ;
    QHBoxLayout *hBoxPointSize=new QHBoxLayout() ;
    hBoxPointSize->addWidget( lblWmMoins ) ;
    hBoxPointSize->addWidget( m_wmPointSizeFontSlider ) ;
    hBoxPointSize->addWidget( lblWmPlus ) ;
    vBoxPointSize->addWidget( lblWmPointSize ) ;
    vBoxPointSize->addLayout( hBoxPointSize ) ;
    vBoxPointSize->addStretch( 1 ) ;

    QHBoxLayout *hBoxVibration=new QHBoxLayout() ;
    hBoxVibration->addWidget( lblVibration ) ;
    hBoxVibration->addWidget( m_checkBoxActivateVibration ) ;

    QHBoxLayout *hBoxSleepTh=new QHBoxLayout() ;
    hBoxSleepTh->addWidget( lblSleepThread ) ;
    hBoxSleepTh->addWidget( m_checkBoxActivateSleepThread ) ;
    
    QHBoxLayout *hBoxWmInfo=new QHBoxLayout() ;
    hBoxWmInfo->addWidget( m_labelWmWorks ) ;

    QVBoxLayout *vBox=new QVBoxLayout() ;
    vBox->addLayout( vBoxSens ) ;
    vBox->addSpacing( 30 ) ;
    vBox->addLayout( vBoxPointSize ) ;
    vBox->addSpacing( 30 ) ;
    vBox->addLayout( hBoxVibration ) ;
    vBox->addSpacing( 30 ) ;
    vBox->addLayout( hBoxSleepTh ) ;
    vBox->addSpacing( 30 ) ;
    vBox->addLayout( hBoxWmInfo ) ;
    vBox->addStretch( 1 ) ;

    m_settingsWidget->setLayout( vBox ) ;
  }
}


void SettingsWidget::resetWidget()
{
  if( m_irSensitiveSlider != NULL )
    m_irSensitiveSlider->setValue( PLUGIN_WM_SENSITIVE_DEFAULT ) ;

  if( m_wmPointSizeFontSlider != NULL )
    m_wmPointSizeFontSlider->setValue( (int)(WMTOOL_POINTSIZE_RATIO_DEFAULT * 10.0f) ) ;

  if( m_checkBoxActivateVibration != NULL )
    m_checkBoxActivateVibration->setChecked(PLUGIN_WM_VIBRATION_ONOFF) ;

  if( m_checkBoxActivateSleepThread != NULL )
    m_checkBoxActivateSleepThread->setChecked(PLUGIN_WM_SLEEPTHREAD_ONOFF) ;
    
  if( m_labelWmWorks != NULL )
    m_labelWmWorks->setText( m_wmWorks ) ;
}
