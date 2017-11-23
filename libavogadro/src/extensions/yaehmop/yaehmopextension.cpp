/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2016 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include <avogadro/molecule.h>
#include <avogadro/atom.h>

#include <avogadro/plotwidget.h>
#include <avogadro/plotobject.h>
#include <avogadro/plotaxis.h>
#include <avogadro/plotpoint.h>

#include <openbabel/generic.h>
#include <openbabel/mol.h>

#include <QAction>
#include <QCoreApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLayout>
#include <QList>
#include <QMessageBox>
#include <QPair>
#include <QProcess>
#include <QString>
#include <QTextEdit>
#include <QVector3D>

#include "numvalenceelectrons.h"
#include "yaehmopbanddialog.h"
#include "yaehmopextension.h"
#include "yaehmoptotaldosdialog.h"
#include "yaehmopout.h"

using namespace std;
using namespace Eigen;

namespace Avogadro
{
  enum YaehmopExtensionIndex
  {
    ActionIndex = 0,
    SeparatorIndex
  };

  YaehmopExtension::YaehmopExtension(QObject *parent)
    : Extension(parent),
      m_bandNumKPoints(40),
      m_dosKPoints("15x15x15"),
      m_useSmoothing(true),
      m_eStep(0.1),
      m_broadening(0.1),
      m_displayData(false),
      m_limitY(false),
      m_minY(0.0),
      m_maxY(0.0),
      m_plotFermi(false),
      m_fermi(0.0),
      m_zeroFermi(false),
      m_numDimensions(3)
  {
    QAction *action = new QAction( this );
    action->setSeparator(true);
    action->setData(SeparatorIndex);
    m_actions.append(action);

    // create an action for our first action
    action = new QAction( this );
    action->setText(tr("Calculate Band Structure..."));
    action->setData(ActionIndex);
    m_actions.append(action);
    connect(action, SIGNAL(triggered()), SLOT(calculateBandStructure()));

    // create an action for our second action
    action = new QAction(this);
    action->setText(tr("Calculate Total Density of States..."));
    action->setData(ActionIndex);
    m_actions.append(action);
    connect(action, SIGNAL(triggered()), SLOT(calculateTotalDOS()));

    action = new QAction( this );
    action->setText(tr("Set Parameters File..."));
    action->setData(ActionIndex);
    m_actions.append(action);
    connect(action, SIGNAL(triggered()), SLOT(setParametersFile()));

    action = new QAction( this );
    action->setText(tr("Execute Custom Input..."));
    action->setData(ActionIndex);
    m_actions.append(action);
    connect(action, SIGNAL(triggered()), SLOT(executeCustomInput()));
  }

  YaehmopExtension::~YaehmopExtension()
  {
  }

  QList<QAction *> YaehmopExtension::actions() const
  {
    return m_actions;
  }

  QString YaehmopExtension::menuPath(QAction *action) const
  {
    int i = action->data().toInt();

    if (i == SeparatorIndex)
      return QString();

    return tr("E&xtensions") + '>' + tr("&Yaehmop");
  }

  QDockWidget * YaehmopExtension::dockWidget()
  {
    return 0;
  }

  // Get the distance between two k points
  static inline double distance(const kpoint& a, const kpoint& b)
  {
    return sqrt(pow(a[0] - b[0], 2.0) +
                pow(a[1] - b[1], 2.0) +
                pow(a[2] - b[2], 2.0));
  };

  // Get the distance between two special k points
  static inline double distance(const specialKPoint& a,
                                const specialKPoint& b)
  {
    return distance(a.coords, b.coords);
  }

  void YaehmopExtension::writeSettings(QSettings &settings) const
  {
    settings.beginGroup("yaehmopextension");

    settings.beginGroup("generic");
    settings.setValue("displayData", m_displayData);
    settings.setValue("limitY", m_limitY);
    settings.setValue("minY", m_minY);
    settings.setValue("maxY", m_maxY);
    settings.setValue("zeroFermi", m_zeroFermi);
    settings.setValue("numDimensions", m_numDimensions);
    settings.endGroup();

    settings.beginGroup("bandStructure");
    settings.setValue("bandNumKPoints",
                      static_cast<unsigned long long>(m_bandNumKPoints));
    settings.setValue("plotFermi", m_plotFermi);
    settings.setValue("fermi", m_fermi);
    settings.endGroup();

    settings.beginGroup("DOS");
    settings.setValue("dosKPoints", m_dosKPoints);
    settings.setValue("useSmoothing", m_useSmoothing);
    settings.setValue("eStep", m_eStep);
    settings.setValue("broadening", m_broadening);
    settings.endGroup();

    settings.endGroup();
  }

  void YaehmopExtension::readSettings(QSettings &settings)
  {
    settings.beginGroup("yaehmopextension");

    settings.beginGroup("generic");
    m_displayData = settings.value("displayData", m_displayData).toBool();
    m_limitY = settings.value("limitY", m_limitY).toBool();
    m_minY = settings.value("minY", m_minY).toDouble();
    m_maxY = settings.value("maxY", m_minY).toDouble();
    m_zeroFermi = settings.value("zeroFermi", m_zeroFermi).toBool();
    m_numDimensions = settings.value("numDimensions", m_numDimensions).toUInt();
    settings.endGroup();

    settings.beginGroup("bandStructure");
    m_bandNumKPoints =
      settings.value("bandNumKPoints",
                     static_cast<unsigned long long>(m_bandNumKPoints))
                       .toULongLong();
    m_plotFermi = settings.value("plotFermi", m_plotFermi).toBool();
    m_fermi = settings.value("fermi", m_fermi).toDouble();
    settings.endGroup();

    settings.beginGroup("DOS");
    m_dosKPoints = settings.value("dosKPoints", m_dosKPoints).toString();
    m_useSmoothing = settings.value("useSmoothing", m_useSmoothing).toBool();
    m_eStep = settings.value("eStep", m_eStep).toDouble();
    m_broadening = settings.value("broadening", m_broadening).toDouble();
    settings.endGroup();

    settings.endGroup();
  }

  // Returns false if an error was found
  bool YaehmopExtension::checkForErrors(const QString& output, QString& error)
  {
    QStringList lines = output.split(QRegExp("[\r\n]"),
                                     QString::SkipEmptyParts);
    bool zhegvErrorFound = false;

    for (int i = 0; i < lines.size(); ++i) {
      // This usually means that ZHEGV failed
      if (lines[i].contains("ERROR: Problems in the diagonalization, try") &&
          !zhegvErrorFound) {
        error += "Error during diagonalization. ZHEGV returned with an error.";
        error += "\nTry changing the k points.\n";
        zhegvErrorFound = true;
      }
    }

    if (zhegvErrorFound)
      return false;
    return true;
  }

  void YaehmopExtension::calculateBandStructure()
  {
    QString input = createYaehmopBandInput();
    // If the input is empty, either the user cancelled
    // or an error box has already popped up...
    if (input.isEmpty())
      return;

    QString output;

    // Execute Yaehmop
    executeYaehmop(input, output);

    QString error;
    if (!checkForErrors(output, error)) {
      QMessageBox::warning(NULL,
                        tr("Yaehmop"),
                        tr((QString("Data may be invalid. "
                                    "Error received in calculation:\n")
                            + error).toStdString().c_str()));
      qDebug() << "Full Yaehmop output is as follows:\n" << output;
      qDebug() << "Data may be invalid. Error received in calculation:\n" + error;
    }

    // qDebug() << "Output is:";
    // qDebug() << output;

    // Trim the output so it only contains the band data
    // Remove everything before BAND_DATA
    int ind = output.indexOf("BAND_DATA");
    if (ind == -1) {
      qDebug() << "Error in " << __FUNCTION__ << ": band data not found in"
               << "Yaehmop output!";
      return;
    }
    output.remove(0, ind - 1);

    // Remove everything after END_BAND_DATA
    ind = output.indexOf("END_BAND_DATA");
    if (ind == -1) {
      qDebug() << "Error in " << __FUNCTION__ << ": band data did not"
               << "complete in Yaehmop output!";
      return;
    }
    output.remove(ind + QString("END_BAND_DATA\n").size(), output.size());

    //qDebug() << "After trimming, output is now:";
    //qDebug() << output;

    QVector<band> bands;
    QVector<kpoint> kpoints;
    QVector<specialKPoint> specialKPoints;

    if (!YaehmopOut::readBandData(output, bands, kpoints, specialKPoints) ||
        bands.size() == 0 || kpoints.size() == 0) {
      qDebug() << "Error in " << __FUNCTION__ << ": failed to read band data!";
      return;
    }

    int numKPoints = kpoints.size();
    int numOrbitals = bands.size();
    int numSpecialKPoints = specialKPoints.size();

    // If there is only one special k point, there is nothing to graph. Just
    // return.
    if (numSpecialKPoints <= 1) {
      qDebug() << "Error in " << __FUNCTION__ << ": we need two or more "
               << "special k points!";
      return;
    }

    //qDebug() << "numKPoints is " << numKPoints;
    //qDebug() << "numOrbitals is " << numOrbitals;
    //qDebug() << "numSpecialKPoints is " << numSpecialKPoints;

    // These values are close to the limits of doubles
    double min_y = 1e300, max_y = -1e300;
    double min_x = 0.0, max_x = 0.0;

    // Points in each inner vector of bands
    QVector<QVector<QPointF> > points;

    for (int i = 0; i < numOrbitals; ++i) {
      QVector<QPointF> energies;
      // Keep track of the distance we have gone thus far
      double distanceSoFar = 0.0;
      for (int j = 0; j < numKPoints; ++j) {
        if (j != 0)
          distanceSoFar += distance(kpoints[j - 1], kpoints[j]);

        // x is k-point distance so far. y is energy
        double x = distanceSoFar;
        double y = (m_zeroFermi ? bands[i][j] - m_fermi : bands[i][j]);
        energies.append(QPointF(x, y));

        // Correct the min_y and max_y values
        if (y < min_y)
          min_y = y;
        if (y > max_y)
          max_y = y;

        if (distanceSoFar > max_x)
          max_x = distanceSoFar;
      }
      points.append(energies);
    }

    // These are for k point labels
    QList<double> kpointlabels_x;
    QStringList kpointlabels;
    double distanceSoFar = 0.0;
    for (int i = 0; i < numSpecialKPoints; ++i) {
      if (i != 0)
        distanceSoFar += distance(specialKPoints[i - 1], specialKPoints[i]);

      // If we have it set exactly on the left or right axis, it won't
      // plot them. Move them just a tiny bit so that it will.
      if (i == 0)
        kpointlabels_x.append(1e-4);
      else if (i == numSpecialKPoints - 1)
        kpointlabels_x.append(max_x - 1e-4);
      else {
        kpointlabels_x.append(distanceSoFar);
      }
      kpointlabels.append(specialKPoints[i].label);
    }

    PlotWidget *pw = new PlotWidget;
    pw->setWindowTitle(tr("Yaehmop Band Structure"));

    // Let's make our widget a reasonable size
    pw->resize(500, 500);

    // Double the font size
    pw->setFontSize(16);

    // Set our limits for the plot
    // If we are limiting y, then change min_y and max_y
    if (m_limitY) {
      min_y = m_minY;
      max_y = m_maxY;
    }
    pw->setDefaultLimits(min_x, max_x, min_y, max_y);

    // Set up our axes
    pw->axis(PlotWidget::BottomAxis)->setTickCustomStrings(kpointlabels_x,
                                                           kpointlabels);
    pw->axis(PlotWidget::LeftAxis)->setLabel(tr("Energy (eV)"));

    // White background
    pw->setBackgroundColor(Qt::white);
    pw->setForegroundColor(Qt::black);

    // Add the objects
    for (int i = 0; i < numOrbitals; ++i) {
      PlotObject *po = new PlotObject(Qt::red, PlotObject::Lines);
      QPen linePen(po->linePen());
      linePen.setWidth(2);
      po->setLinePen(linePen);
      for (int j = 0; j < numKPoints; ++j) {
        po->addPoint(points[i][j].x(), points[i][j].y());
      }
      // Add the object to the widget
      pw->addPlotObject(po);
    }

    // If we have the fermi energy, plot that as a dashed line
    if (m_plotFermi) {
      double tempFermi = (m_zeroFermi ? 0 : m_fermi);
      size_t range = 75;
      for (size_t i = 0; i < range; i += 2) {
        PlotObject *tempPo = new PlotObject(Qt::black, PlotObject::Lines);
        double newRange = max_x - min_x;
        double x1 = (((i - min_x) * newRange) / range) + min_x;
        double x2 = (((i + 1 - min_x) * newRange) / range) + min_x;
        tempPo->addPoint(QPointF(x1, tempFermi));
        tempPo->addPoint(QPointF(x2, tempFermi));
        pw->addPlotObject(tempPo);
      }
    }

    // If we are to display band data, show that first
    if (m_displayData) {
      QString bandDataStr;

      // Show number of dimensions first
      bandDataStr += QString("# Number of dimensions: ") +
                     QString::number(m_numDimensions) + "\n";

      // Add in special k point info
      bandDataStr += "# Special k points\n";
      bandDataStr += "# <symbol> <x> <y> <z> <k space distance (x)>\n";

      for (int i = 0; i < specialKPoints.size(); ++i) {
        bandDataStr += (QString("# ") + specialKPoints[i].label + " ");
        for (int j = 0; j < 3; ++j)
          bandDataStr += (QString().sprintf("%6.2f",
                                            specialKPoints[i].coords[j]) + " ");
        bandDataStr += (QString().sprintf("%6.2f", kpointlabels_x[i]) + "\n");
      }

      // Now add in the actual data
      bandDataStr += ("\n# <k space distance (x)> <band 1 energies> "
                      "<band 2 energies> <etc.>\n");

      distanceSoFar = 0.0;
      for (int i = 0; i < numKPoints; ++i) {
        if (i != 0)
          distanceSoFar += distance(kpoints[i - 1], kpoints[i]);
        double x = distanceSoFar;
        bandDataStr += (QString().sprintf("%10.6f", x) + " ");
        for (int j = 0; j < numOrbitals; ++j) {
          // Unfortunately, these are accessed out of order here...
          bandDataStr += (QString().sprintf("%10.6f", bands[j][i]) + " ");
        }
        bandDataStr += "\n";
      }

      // Done! Let's make the dialog and show it.
      QDialog* dialog = new QDialog;
      QVBoxLayout* layout = new QVBoxLayout;
      dialog->setLayout(layout);
      dialog->setWindowTitle(tr("Yaehmop Band Data Results"));
      QTextEdit* edit = new QTextEdit;
      layout->addWidget(edit);
      dialog->resize(500, 500);

      // Show the user the output
      edit->setText(bandDataStr);

      // Make sure this gets deleted upon closing
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    }

    // Let's make sure this gets deleted when we close it
    pw->setAttribute(Qt::WA_DeleteOnClose);
    // Show it!
    pw->show();
  }

  void YaehmopExtension::calculateTotalDOS()
  {
    QString input = createYaehmopTotalDOSInput();
    // If the input is empty, either the user cancelled
    // or an error box has already popped up...
    if (input.isEmpty())
      return;

    QString output;

    // Execute Yaehmop
    if (!executeYaehmop(input, output)) {
      qDebug() << "Error while executing Yaehmop in " <<  __FUNCTION__;
      return;
    }

    QString error;
    if (!checkForErrors(output, error)) {
      QMessageBox::warning(NULL,
                        tr("Yaehmop"),
                        tr((QString("Data may be invalid. "
                                    "Error received in calculation:\n")
                            + error).toStdString().c_str()));
      qDebug() << "Full Yaehmop output is as follows:\n" << output;
      qDebug() << "Data may be invalid. Error received in calculation:\n" + error;
    }

    //qDebug() << "input is " << input;
    //qDebug() << "output is " << output;

    // First, let's get the fermi energy
    bool fermiFound = true;
    double unadjustedFermi, fermi = 0.0;
    if (!YaehmopOut::getFermiLevelFromDOSData(output, unadjustedFermi)) {
      qDebug() << "Fermi level could not be obtained in " << __FUNCTION__;
      fermiFound = false;
    }

    if (!m_zeroFermi)
      fermi = unadjustedFermi;

    // Trim the output so it only contains the total DOS
    // Remove everything before TOTAL DENSITY OF STATES
    int ind = output.indexOf("TOTAL DENSITY OF STATES");
    if (ind == -1) {
      qDebug() << "Error in " << __FUNCTION__ << ": total DOS data not found in"
               << "Yaehmop output!";
      return;
    }
    output.remove(0, ind - 1);

    // Remove everything after END OF DOS
    ind = output.indexOf("END OF DOS");
    if (ind == -1) {
      qDebug() << "Error in " << __FUNCTION__ << ": total DOS data did not"
               << "complete in Yaehmop output!";
      return;
    }
    output.remove(ind + QString("END OF DOS\n").size(), output.size());

    //qDebug() << "After trimming, output is now:";
    //qDebug() << output;

    QVector<double> densities;
    QVector<double> energies;

    if (!YaehmopOut::readTotalDOSData(output, densities, energies) ||
        densities.size() == 0 || energies.size() != densities.size()) {
      qDebug() << "Error in " << __FUNCTION__
               << ": failed to read total DOS data!";
      return;
    }

    // If we need to zero the Fermi energy, go ahead and do that
    if (fermiFound && m_zeroFermi) {
      for (int i = 0; i < energies.size(); ++i)
        energies[i] -= unadjustedFermi;
    }

    // If we smooth data, this will be calculated
    QList<double> integration;

    // Let's smooth the data if we need to
    if (m_useSmoothing) {
      smoothData(densities, energies, m_eStep, m_broadening);

      // Let's get the integration data as well
      // This assumes uniform spacing between the energy levels
      double xDiff = (energies.size() > 1 ? energies[1] - energies[0]: 0.0);
      integration = integrateDataTrapezoidal(xDiff, densities);
    }

    // Plotting is fairly simple - densities on x axis and energies on y
    // These values are close to the limits of doubles
    double min_y = 1e300, max_y = -1e300;
    double min_x = 0.0, max_x = -1e300;

    for (int i = 0; i < densities.size(); ++i) {
      // Correct the max_x, min_y, and max_y
      if (densities[i] > max_x)
        max_x = densities[i];
      if (energies[i] < min_y)
        min_y = energies[i];
      if (energies[i] > max_y)
        max_y = energies[i];
    }

    PlotWidget *pw = new PlotWidget;
    pw->setWindowTitle(tr("Yaehmop Total DOS"));

    // Let's make our widget a reasonable size
    pw->resize(500, 500);

    // Double the font size
    pw->setFontSize(16);

    // Set our limits for the plot
    // If we are limiting y, then change min_y and max_y
    if (m_limitY) {
      min_y = m_minY;
      max_y = m_maxY;
    }
    pw->setDefaultLimits(min_x, max_x, min_y, max_y);

    // Set up our axes
    pw->axis(PlotWidget::BottomAxis)->setLabel(tr("Density of States"));
    pw->axis(PlotWidget::LeftAxis)->setLabel(tr("Energy (eV)"));

    // White background
    pw->setBackgroundColor(Qt::white);
    pw->setForegroundColor(Qt::black);

    // Add the objects
    PlotObject *po = new PlotObject(Qt::red, PlotObject::Lines);
    QPen linePen(po->linePen());
    linePen.setWidth(2);
    po->setLinePen(linePen);

    // Add an extra point at the beginning to make it go to the y axis
    if (!energies.empty())
      po->addPoint(QPointF(0, energies.front()));

    for (int i = 0; i < densities.size(); ++i)
      po->addPoint(QPointF(densities[i], energies[i]));

    // Add an extra point at the end to make it go to the y axis
    if (!energies.empty())
      po->addPoint(QPointF(0, energies.back()));

    // If we have the fermi energy, plot that as a dashed line
    if (fermiFound) {
      size_t num = 75;
      for (size_t i = 0; i < num; i += 2) {
        PlotObject *tempPo = new PlotObject(Qt::black, PlotObject::Lines);
        tempPo->addPoint(QPointF(static_cast<double>(i) /
                                 static_cast<double>(num) *
                                 static_cast<double>(max_x), fermi));
        tempPo->addPoint(QPointF(static_cast<double>(i + 1) /
                                 static_cast<double>(num) *
                                 static_cast<double>(max_x), fermi));
        pw->addPlotObject(tempPo);

      }
      // Also set the unadjusted fermi level in memory
      m_fermi = unadjustedFermi;
    }

    // If we have the integration data, plot that as well. Make it blue.
    if (integration.size() != 0) {
      double maxVal = integration.back();
      PlotObject *tempPo = new PlotObject(Qt::blue, PlotObject::Lines);
      QPen linePen(po->linePen());
      linePen.setWidth(2);
      po->setLinePen(linePen);
      for (int i = 0; i < integration.size(); ++i) {
        tempPo->addPoint(QPointF(integration[i] / maxVal * max_x,
                                 energies[i]));
      }
      pw->addPlotObject(tempPo);
      // Now let's add a label for it and use secondary axes
      pw->setSecondaryLimits(0, qRound(maxVal), min_y, max_y);
      pw->axis(PlotWidget::TopAxis)->setLabel(tr("Integration (# electrons)"));
      pw->axis(PlotWidget::TopAxis)->setVisible(true);
      pw->axis(PlotWidget::TopAxis)->setTickLabelsShown(true);
      pw->setTopPadding(60);
    }

    pw->addPlotObject(po);
    pw->setAttribute(Qt::WA_DeleteOnClose);

    // If we are to display band data, show that first
    if (m_displayData) {
      QString DOSDataStr;

      // Show number of dimensions first
      DOSDataStr += QString("# Number of dimensions: ") +
                    QString::number(m_numDimensions) + "\n";

      // Let's print the fermi energy
      if (fermiFound)
        DOSDataStr += QString("# Unadjusted Fermi level: ") +
                      QString::number(unadjustedFermi) + "\n";
      else
        DOSDataStr += "# Fermi level not found!\n";

      // Add in k point info first
/*
      DOSDataStr += "\n# k points\n";
      DOSDataStr += "# <x> <y> <z> <weight>\n";

      QStringList inputSplit = input.split(QRegExp("[\r\n]"),
                                           QString::SkipEmptyParts);

      while (inputSplit.size() != 0 && !inputSplit[0].contains("k points"))
        inputSplit.removeFirst();

      for (size_t i = 0; i < inputSplit.size(); ++i) {
        if (inputSplit[i].split(QRegExp(" "),
                                QString::SkipEmptyParts).size() == 0) {
          break;
        }
        if (inputSplit[i].split(QRegExp(" "),
                                QString::SkipEmptyParts).size() != 4) {
          continue;
        }
        DOSDataStr += QString("# ") + inputSplit[i] + "\n";
      }
*/
      // Now for the actual data
      DOSDataStr += "\n# <density (x)> <energy (y)>\n";

      for (int i = 0; i < densities.size(), i < energies.size(); ++i) {
        DOSDataStr += (QString().sprintf("%10.6f", densities[i]) + " " +
                       QString().sprintf("%10.6f", energies[i]) + "\n");
      }

      // If we have integration data, add that too
      if (integration.size() != 0) {
        DOSDataStr += "\n\n# Integration Data:\n";
        DOSDataStr += "\n# <integration> <energies>\n";
        for (int i = 0; i < energies.size(), i < integration.size(); ++i) {
          DOSDataStr += (QString().sprintf("%10.6f", integration[i]) + " " +
                         QString().sprintf("%10.6f", energies[i]) + "\n");
        }
      }

      // Done! Let's make the dialog and show it.
      QDialog* dialog = new QDialog;
      QVBoxLayout* layout = new QVBoxLayout;
      dialog->setLayout(layout);
      dialog->setWindowTitle(tr("Yaehmop Total DOS Results"));
      QTextEdit* edit = new QTextEdit;
      layout->addWidget(edit);
      dialog->resize(500, 500);

      // Show the user the output
      edit->setText(DOSDataStr);

      // Make sure this gets deleted upon closing
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->show();
    }

    // Show the plot!
    pw->show();
  }

  QString YaehmopExtension::createYaehmopBandInput()
  {
    if (!m_molecule) {
      qDebug() << "Error in " << __FUNCTION__ << ": the molecule is not set";
      return "";
    }

    OpenBabel::OBUnitCell *cell = m_molecule->OBUnitCell();
    if (!cell) {
      QMessageBox::warning(NULL,
                        tr("Avogadro"),
                        tr("Cannot calculate band structure: no unit cell!"));
      qDebug() << "Error in " << __FUNCTION__ << ": there is no unit cell";
      return "";
    }

    // Let's get the k-point info from the user first so we don't have to run
    // through the rest of the algorithm if they cancel...
    QString specialKPointString;
    YaehmopBandDialog d;
    if (!d.getUserOptions(m_molecule, m_bandNumKPoints, specialKPointString,
                          m_displayData, m_limitY, m_minY, m_maxY, m_plotFermi,
                          m_fermi, m_zeroFermi, m_numDimensions))
      return "";

    // Proceed with the function
    QString input;
    input += "Title\n"; // Title
    input += createGeometryAndLatticeInput();

    // Here we describe the number of k points connecting each special
    // k point, the number of special k points, and their locations
    // in reciprocal space. This is something we will let the user change
    input += "Band\n";

    // This is the number of kpoints connecting each special k point
    input += (QString::number(m_bandNumKPoints) + "\n");
    // Num special k points
    int numSK = specialKPointString.split(QRegExp("[\r\n]"),
                                          QString::SkipEmptyParts).size();
    input += (QString::number(numSK) + "\n"); // num special k points
    input += specialKPointString; // Add the whole string from user input

    // And we are done!
    // qDebug() << "Input is:";
    // qDebug() << input;

    return input;
  }

  QString YaehmopExtension::createYaehmopTotalDOSInput()
  {
    if (!m_molecule) {
      qDebug() << "Error in " << __FUNCTION__ << ": the molecule is not set";
      return "";
    }

    OpenBabel::OBUnitCell *cell = m_molecule->OBUnitCell();
    if (!cell) {
      QMessageBox::warning(NULL,
                           tr("Avogadro"),
                           tr("Cannot calculate total DOS: no unit cell!"));
      qDebug() << "Error in " << __FUNCTION__ << ": there is no unit cell";
      return "";
    }

    // Let's get the k points from the user first so we don't have to run
    // through the rest of the algorithm if they cancel...
    QList<Atom*> atoms = m_molecule->atoms();
    std::vector<unsigned char> atomicNums;
    for (int i = 0; i < atoms.size(); ++i)
      atomicNums.push_back(atoms[i]->atomicNumber());
    size_t numValElectrons = numValenceElectrons(atomicNums);
    size_t numKPoints = 0;
    QString tempDOSKPoints = m_dosKPoints;
    YaehmopTotalDOSDialog d;
    if (!d.getUserOptions(this, numValElectrons, numKPoints,
                          tempDOSKPoints, m_displayData, m_useSmoothing,
                          m_eStep, m_broadening, m_limitY,
                          m_minY, m_maxY, m_zeroFermi, m_numDimensions)) {
      return "";
    }

    // Proceed with the function
    QString input;
    input += "Title\n"; // Title

    // Crystal geometry
    input += createGeometryAndLatticeInput();

    // Total DOS is calculated in an average properties calculation
    input += "average properties\n";

    // According to the manual, we can save a lot of time by avoiding
    // calculations of extraneous data if we use this keyword
    input += "Just Average E\n";

    // Now we need to input the number of valence electrons
    input += "electrons\n";
    input += (QString::number(numValElectrons) + "\n");

    // k points!
    input += "k points\n";
    input += (QString::number(numKPoints) + "\n");
    input += tempDOSKPoints;

    // We're done!
    return input;
  }

  void YaehmopExtension::setParametersFile()
  {
    QString dialogText = tr("Select the Yaehmop Parameters File");

    QString fileName = QFileDialog::getOpenFileName(
         NULL,
         dialogText,
         QDir::homePath(),
         tr("Dat files (*.dat);;All files (*.*)"));

    if (fileName.isNull())
      return;

    m_parametersFile = fileName;
  }

  void YaehmopExtension::executeCustomInput() const
  {
    // Create a dialog where the user can enter their own input
    QDialog dialog;
    QVBoxLayout layout;
    dialog.setLayout(&layout);
    dialog.setWindowTitle(tr("Execute Custom Input"));
    QTextEdit edit;
    layout.addWidget(&edit);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(&buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));
    layout.addWidget(&buttons);
    dialog.resize(500, 500);

    // Show the dialog. Does the user accept?
    if (dialog.exec() != QDialog::Accepted)
      return;

    // Run the input!
    QString input = edit.toPlainText();
    QString output;
    bool success = executeYaehmop(input, output);
    QString successStr = tr("Run completed!");
    if (!success)
      successStr = tr("Run failed!");

    // Remove the button box
    buttons.hide();
    layout.removeWidget(&buttons);

    // Show the user the output
    edit.setText(output);
    dialog.setWindowTitle(successStr);
    dialog.exec();
  }

  bool YaehmopExtension::executeYaehmop(QString input, QString& output) const
  {
#ifdef __APPLE__
    // For apple, find yaehmop relative to the application directory
    QString program = QCoreApplication::applicationDirPath() + "/../bin/yaehmop";
#elif defined _WIN32
    // For Windows, assume yaehmop is in the same directory as avogadro
    QString program = "yaehmop";
#else
    // For Linux, assume yaehmop is in the same directory as well.
    // The following format is needed for packages
    QString program = QCoreApplication::applicationDirPath() + "/yaehmop";
#endif

    QStringList arguments;
    arguments << "--use_stdin_stdout";

    // If the user set a parameters file, let's use that
    if (!m_parametersFile.isEmpty())
      arguments << m_parametersFile;

    QProcess p;
    p.start(program, arguments);

    if (!p.waitForStarted()) {
      QMessageBox::warning(NULL,
                        tr("Avogadro"),
                        tr("Error: Yaehmop failed to start!"));
      qDebug() << "Error: Yaehmop executable at" << program
               << "failed to start.";
      return false;
    }

    // Give it the input!
    p.write(input.toStdString().c_str());

    // Close the write channel
    p.closeWriteChannel();

    if (!p.waitForFinished()) {
      QMessageBox::warning(NULL,
                        tr("Avogadro"),
                        tr("Error: Yaehmop failed to finish!"));
      qDebug() << "Error: Yaehmop executable at" << program
               << "failed to finish.";
      output = p.readAll();
      qDebug() << "Output is as follows:\n" << output;
      return false;
    }

    int exitStatus = p.exitStatus();
    output = p.readAll();

    if (exitStatus == QProcess::CrashExit) {
      QMessageBox::warning(NULL,
                        tr("Avogadro"),
                        tr("Error: Yaehmop crashed!"));
      qDebug() << "Error: Yaehmop crashed!\n";
      qDebug() << "Output is as follows:\n" << output;
      return false;
    }

    if (exitStatus != QProcess::NormalExit) {
      qDebug() << "Error: Yaehmop finished abnormally with exit code "
               << exitStatus;
      qDebug() << "Output is as follows:\n" << output;
      return false;
    }

    // We did it!
    return true;
  }

  QString YaehmopExtension::createGeometryAndLatticeInput() const
  {
    OpenBabel::OBUnitCell *cell = m_molecule->OBUnitCell();
    if (!cell) {
      QMessageBox::warning(NULL,
                        tr("Avogadro"),
                        tr("Cannot calculate band structure: no unit cell!"));
      qDebug() << "Error in " << __FUNCTION__ << ": there is no unit cell";
      return "";
    }

    QList<Avogadro::Atom*> atoms = m_molecule->atoms();

    // This is the minimum number we allow doubles. If a number's float
    // absolute value is smaller than this, we will round it to 0.
    double minNum = 1e-8;

    QString input;
    input += "Geometry\n"; // Begin geometry section
    int numAtoms = atoms.size();
    // Num atoms plus (numDimensions + 1) dummies.
    // Dummies are for defining the lattice
    input += (QString::number(numAtoms + m_numDimensions + 1) + QString("\n"));

    // Now loop through atom positions and add them
    for (int i = 0; i < numAtoms; ++i) {
      QString symbol = OpenBabel::etab.GetSymbol(atoms[i]->atomicNumber());
      const Vector3d& pos = *atoms[i]->pos();
      input += (QString::number(i + 1) + " ");
      input += (symbol + " ");
      for (size_t j = 0; j < 3; ++j)
        // If the position is small, just use 0
        input += (QString::number((fabs(pos[j]) > 1e-8 ? pos[j] : 0)) + " ");
      input += "\n";
    }

    // Get the lattice
    std::vector<OpenBabel::vector3> latticeVecs = cell->GetCellVectors();
    // If the lattice element is very small, just round it to zero
    for (size_t i = 0; i < latticeVecs.size(); ++i) {
      // Unfortunately, OpenBabel::vector3[] does not return a reference...
      // So this is a little bit more lengthy than it could be
      double x = (fabs(latticeVecs[i][0]) > minNum ? latticeVecs[i][0] : 0.0);
      double y = (fabs(latticeVecs[i][1]) > minNum ? latticeVecs[i][1] : 0.0);
      double z = (fabs(latticeVecs[i][2]) > minNum ? latticeVecs[i][2] : 0.0);
      latticeVecs[i] = OpenBabel::vector3(x, y, z);
    }

    // Add the dummy atoms - these tell the program where the lattice is
    for (size_t i = 0; i <= m_numDimensions; ++i) {
      input += (QString::number(numAtoms + i + 1) + " ");
      // Symbol for dummy atoms
      input += "& ";
      // First dummy is at 0,0,0, the other dummies are at the ends of the
      // lattice
      if (i == 0) {
        input += "0 0 0\n";
      }
      else {
        // We only get here if i > 0.
        // i - 1 is equal to the index of the vector we are looking at.
        for (size_t j = 0; j < 3; ++j)
          input += (QString::number(latticeVecs[i - 1][j]) + " ");
        input += "\n";
      }
    }

    // Let's calculate the number of overlaps to use
    // The manual says that numOverlaps * latticeVecLength should be between
    // 10 and 20 Angstroms. Let's always use a numOverlaps of at least 3 and
    // then use more if numOverlaps * latticeVecLength < 20.
    Vector3i overlaps(3,3,3);
    Vector3d latticeLengths(cell->GetA(), cell->GetB(), cell->GetC());

    for (size_t i = 0; i < 3; ++i) {
      while (overlaps[i] * latticeLengths[i] < 20)
        ++overlaps[i];
    }

    // Lattice section to define the lattice
    input += "lattice\n";
    input += QString::number(m_numDimensions) + "\n";
    // Add numbers of overlaps
    for (size_t i = 0; i < m_numDimensions; ++i)
      input += (QString::number(overlaps[i]) + " ");
    input += "\n";
    // If we have "4 5" here, that means the vector is defined
    // from atom 4 to atom 5. We use dummy atoms for this. The first dummy
    // atom (numAtoms + 1) is always at the origin, and the other dummy atoms
    // are at the ends of the a, b, and c axes.
    for (size_t i = 0; i < m_numDimensions; ++i) {
      input += (QString::number(numAtoms + 1) + " " +
                QString::number(numAtoms + i + 2) + "\n");
    }

    return input;
  }

  // This assumes constant spacing between points.
  // It uses the trapezoid rule.
  QList<double> YaehmopExtension::integrateDataTrapezoidal(double xDist,
                                                const QVector<double>& y)
  {
    if (xDist <= 0.0) {
      qDebug() << "Error in " << __FUNCTION__ << ": xDist is less than "
               << "or equal to zero!";
      return QList<double>();
    }

    QList<double> integration;
    for (int i = 0; i < y.size(); ++i) {
      if (i == 0)
        continue;

      // Start with the last number if possible
      double integ = 0.0;
      if (integration.size() != 0)
        integ = integration.last();
      integ += xDist * (y[i] + y[i - 1]) / 2.0;
      integration.append(integ);
    }

    return integration;
  }

  void YaehmopExtension::smoothData(QVector<double>& densities,
                                    QVector<double>& energies,
                                    double stepE, double broadening)
  {
    if (densities.size() == 0) {
      qDebug() << "Error in " << __FUNCTION__ << ": densities is zero!";
      return;
    }

    if (densities.size() != energies.size()) {
      qDebug() << "Error in " << __FUNCTION__ << ": densities and energies "
               << "do not match in size!";
      return;
    }

    QVector<double> finalDensities;
    QVector<double> finalEnergies;

    int numPoints = densities.size();
    double pi = 3.14159265358979;
    // Normalization factor
    double normFact = 2.0 / sqrt(2.0 * pow(broadening, 2.0) * pi);
    double minE = energies[0];
    double maxE = energies[numPoints - 1];
    size_t numSteps = ceil(fabs(maxE - minE) / stepE) + 1;
/*
    qDebug() << "normFact is " << QString::number(normFact);
    qDebug() << "minE is " << QString::number(minE);
    qDebug() << "maxE is " << QString::number(maxE);
    qDebug() << "numSteps is " << QString::number(numSteps);
    qDebug() << "broadening is " << QString::number(broadening);
    qDebug() << "stepE is " << QString::number(stepE);
*/
    // Loop over the new points
    double currE = minE;
    for (size_t i = 0; i < numSteps; ++i, currE += stepE) {
      double density = 0;

      // Loop over all the points in the curve
      for (int j = 0; j < numPoints; ++j){
        double diffE = pow(currE - energies[j], 2.0);
        //qDebug() << "diffE is " << QString::number(diffE);
        if (diffE <= 25.0) {
          density += densities[j] * exp(-diffE / (2 * pow(broadening, 2.0)));
          //qDebug() << "densities[j] is " << QString::number(densities[j]);
          //qDebug() << "exp(-broadening * diffE) is "
          //         << QString::number(exp(-broadening * diffE));
          //qDebug() << "density is NoW " << QString::number(density);
        }
      }
      density *= normFact;
      //qDebug() << "Density is finally:" << QString::number(density);

      finalDensities.append(density);
      finalEnergies.append(currE);
    }
    densities = finalDensities;
    energies = finalEnergies;
  }

  void YaehmopExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* YaehmopExtension::performAction(QAction *action,
                                                GLWidget *widget)
  {
    return 0;
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(yaehmopextension, Avogadro::YaehmopExtensionFactory)

