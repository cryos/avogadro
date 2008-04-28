/**********************************************************************
  SaveDialog - an improved file-save dialog

  Copyright (C) 2008 by Benoit Jacob

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

#ifndef __SAVEDIALOG_H
#define __SAVEDIALOG_H

#include <QFileDialog>

namespace Avogadro {

  class MainWindow;

  /** \class SaveDialog
    *
    * A generic save-dialog. Compared to the standard save-dialog provided by QFileDialog,
    * this class's main benefit is that it automatically sets the default filename suffix
    * based on the currently selected filter. For that, it was necessary to subclass
    * QFileDialog, because we want the default suffix to be updated as soon as the user
    * selects a different filter inside the dialog, for instance so that the confirmation
    * "do you really want to overwrite filename.xyz?" shows the correct completed filename.
    *
    * If a filter specifies more than one suffix, the first one is taken.
    */
  class SaveDialog : public QFileDialog
  {
      Q_OBJECT

    public slots:
      void updateDefaultSuffix();
    
    public:
      /** \param widget the parent window
        * \param windowTitle the title of the window
        * \param defaultDirectory the directory the dialog defaults to, if non-empty.
        * \param defaultFileName the filename the dialog preselects, if non-empty.
        * \param filters the list of filters. For an explanation look at QFileDialog documentation.
        * \param defaultSuffix the suffix to use when a filter doesn't specify any suffix
        *                      (such as "All Files (*.*)").
        */
      SaveDialog(MainWindow *parent,
                 const QString& windowTitle,
                 const QString& defaultDirectory,
                 const QString& defaultFileName,
                 const QStringList& filters,
                 const QString& defaultSuffix);
      virtual ~SaveDialog() {}
  
      /** For documentation on parameters, see the constructor's documentation: SaveDialog().
        *
        * \param selectedFilter the selected filter chosen by the user (e.g. for saving across sessions)
        * \returns the selected filename, or an empty string if the saving was canceled or
        *          otherwise failed.
        */
      static const QString run(MainWindow *parent,
                               const QString& windowTitle,
                               const QString& defaultDirectory,
                               const QString& defaultFileName,
                               const QStringList& filters,
                               const QString &defaultSuffix,
                               QString &selectedFilter);

    private:
      const QString m_defaultSuffix;
  };

} // end namespace Avogadro
#endif
