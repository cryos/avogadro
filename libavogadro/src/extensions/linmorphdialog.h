/**********************************************************************
  LinMorphDialog - dialog for lin morph extension

  Copyright (C) 2008 by Naomi Fox

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


#ifndef LINMORPHDIALOG_H
#define LINMORPHDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QModelIndex>

#include "ui_linmorphdialog.h"

namespace Avogadro
{
  class LinMorphDialog : public QDialog
  {
      Q_OBJECT

    public:
      //! Constructor
      explicit LinMorphDialog( QWidget *parent = 0, Qt::WindowFlags f = 0 );
      //! Desconstructor
      ~LinMorphDialog();

      void setFrame(int i);
      void setFrameCount(int i);

      int fps();

    private:
      Ui::LinMorphDialog ui;
      int m_frameCount;

    public Q_SLOTS:
      void loadFile();
      void savePovSnapshots();

    Q_SIGNALS:
      void fileName(QString filename);
      void snapshotsPrefix(QString ssFullPrefixText);
      void sliderChanged(int i);
      void fpsChanged(int i);
      void frameCountChanged(int i);
      bool loopChanged(int state);
      void back();
      void play();
      void pause();
      void stop();
      void forward();
  };
}

#endif
