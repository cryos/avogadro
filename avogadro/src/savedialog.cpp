/**********************************************************************
  SaveDialog - an improved file-save dialog

  Copyright (C) 2008 by Benoit Jacob

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "savedialog.h"
#include "mainwindow.h"

#include <QPointer>

namespace Avogadro
{
  void SaveDialog::updateDefaultSuffix()
  {
    const QString filter = selectedFilter();
    QString suffix;
    int i = filter.indexOf("*.");
    if(i != -1)
    {
      // FIXME somebody who knows regexps should make this use a QRegExp.
      int j;
      const QString separators(" )");
      for(j = i; j < filter.size() && !separators.contains(filter[j]); j++) {}
      if(j < filter.size())
      {
        suffix = filter.mid(i+2, j-i-2);
      }
    }
    if(suffix.isEmpty()) suffix = m_defaultSuffix;
    setDefaultSuffix(suffix);
    emit currentChanged(selectedFiles().first());
  }

  SaveDialog::SaveDialog(MainWindow *widget,
                         const QString& windowTitle,
                         const QString& defaultDirectory,
                         const QString& defaultFileName,
                         const QStringList& filters,
                         const QString& defaultSuffix)
    : QFileDialog(widget), m_defaultSuffix(defaultSuffix)
  {
    setWindowTitle(windowTitle);
    if(!(defaultDirectory.isEmpty())) setDirectory(defaultDirectory);
    if(!(defaultFileName.isEmpty())) selectFile(defaultFileName);
    setFilters(filters);
    setFileMode(QFileDialog::AnyFile);
    setAcceptMode(QFileDialog::AcceptSave);
    setConfirmOverwrite(true);
    setLabelText(QFileDialog::Accept, tr("Save"));
    connect(this, SIGNAL(filterSelected(const QString &)), this, SLOT(updateDefaultSuffix()));
    updateDefaultSuffix();
  }

  const QString SaveDialog::run(MainWindow *widget,
                                const QString& windowTitle,
                                const QString& defaultDirectory,
                                const QString& defaultFileName,
                                const QStringList& filters,
                                const QString& defaultSuffix,
                                QString &defaultFilter)
  {
    QString result;

    // Make sure we always have something for a file name
    QString fileName(defaultFileName);
    if (fileName.isEmpty())
      fileName = tr("untitled");

    // we cannot current save to a .gz file, so remove that suffix
    int gzipIndex = fileName.lastIndexOf(QLatin1String(".gz"), -1, Qt::CaseInsensitive);
    if (gzipIndex != -1) {
      fileName.remove(gzipIndex, 3);
    }

#if defined (Q_WS_MAC) || defined (Q_WS_WIN)
// The Mac and Windows Qt/Native dialog already update extensions for us.
// So we'll call the static version.
    result = QFileDialog::getSaveFileName(widget,
                                          windowTitle,
                                          defaultDirectory + '/' + fileName,
                                          filters.join(";;"), &defaultFilter);
#else
    QPointer<SaveDialog> dialog = new SaveDialog(widget, windowTitle, defaultDirectory, fileName, filters, defaultSuffix);
    dialog->selectFilter(defaultFilter);
    if(dialog->exec())
    {
      result = dialog->selectedFiles().first();
      defaultFilter = dialog->selectedFilter();
    }
    delete dialog;
#endif
    return result;
  }

} // end namespace Avogadro

#include "savedialog.moc"
