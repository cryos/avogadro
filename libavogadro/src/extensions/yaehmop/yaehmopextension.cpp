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
#include <QApplication>
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

#include "yaehmopbanddialog.h"
#include "yaehmopextension.h"
#include "yaehmopout.h"

using namespace std;
using namespace Eigen;

namespace Avogadro
{

  YaehmopExtension::YaehmopExtension(QObject *parent) : Extension(parent)
  {
    // create an action for our first action
    QAction *action = new QAction( this );
    action->setText(tr("Calculate Band Structure"));
    m_actions.append(action);
    connect(action, SIGNAL(triggered()), SLOT(calculateBandStructure()));

// These will be added soon!
/*
    // create an action for our second action
    action = new QAction(this);
    action->setText(tr("Plot Total Density of States"));
    m_actions.append(action);
    connect(action, SIGNAL(triggered()), SLOT(plotTotalDOS()));

    // create an action for our third action
    action = new QAction( this );
    action->setText( tr("Plot Partial Density of States"));
    m_actions.append(action);
    connect(action, SIGNAL(triggered()), SLOT(plotPartialDOS()));
*/
    action = new QAction( this );
    action->setText( tr("Set Parameters File"));
    m_actions.append(action);
    connect(action, SIGNAL(triggered()), SLOT(setParametersFile()));

    action = new QAction( this );
    action->setText(tr("Execute Custom Input"));
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
    return tr("E&xtensions") + '>' + tr("Yaehmop");
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

  void YaehmopExtension::calculateBandStructure() const
  {
    // This boolean will be set to true if we are to display band data
    bool displayBandData = false;
    QString input = createYaehmopBandInput(displayBandData);
    // If the input is empty, either the user cancelled
    // or an error box has already popped up...
    if (input.isEmpty())
      return;

    QString output;

    // Execute Yaehmop
    executeYaehmop(input, output);

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

    size_t numKPoints = kpoints.size();
    size_t numOrbitals = bands.size();
    size_t numSpecialKPoints = specialKPoints.size();

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

    for (size_t i = 0; i < numOrbitals; ++i) {
      QVector<QPointF> energies;
      // Keep track of the distance we have gone thus far
      double distanceSoFar = 0.0;
      for (size_t j = 0; j < numKPoints; ++j) {
        if (j != 0)
          distanceSoFar += distance(kpoints[j - 1], kpoints[j]);

        // x is k-point distance so far. y is energy
        double x = distanceSoFar;
        double y = bands[i][j];
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
    for (size_t i = 0; i < numSpecialKPoints; ++i) {
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
    pw->setWindowTitle("Yaehmop Band Structure");

    // Let's make our widget a reasonable size
    pw->resize(500, 500);

    // setting our limits for the plot
    pw->setDefaultLimits(min_x, max_x, min_y, max_y);

    // Set up our axes
    pw->axis(PlotWidget::BottomAxis)->setTickCustomStrings(kpointlabels_x, kpointlabels);
    pw->axis(PlotWidget::LeftAxis)->setLabel(tr("Energy (eV)"));

    // White background
    pw->setBackgroundColor(Qt::white);
    pw->setForegroundColor(Qt::black);

    // Add the objects
    for (size_t i = 0; i < numOrbitals; ++i) {
      PlotObject *po = new PlotObject(Qt::red, PlotObject::Lines);
      for (size_t j = 0; j < numKPoints; ++j) {
        po->addPoint(points[i][j].x(), points[i][j].y());
      }
      // Add the object to the widget
      pw->addPlotObject(po);
    }

    // If we are to display band data, show that first
    if (displayBandData) {
      QString bandDataStr;

      // Add in special k point info first
      bandDataStr += "# Special k points\n";
      bandDataStr += "# <symbol> <x> <y> <z> <k space distance (x)>\n";

      for (size_t i = 0; i < specialKPoints.size(); ++i) {
        bandDataStr += (QString("# ") + specialKPoints[i].label + " ");
        for (size_t j = 0; j < 3; ++j)
          bandDataStr += (QString().sprintf("%6.2f",
                                            specialKPoints[i].coords[j]) + " ");
        bandDataStr += (QString().sprintf("%6.2f", kpointlabels_x[i]) + "\n");
      }

      // Now add in the actual data
      bandDataStr += ("\n# <k space distance (x)> <band 1 energies> "
                      "<band 2 energies> <etc.>\n");

      distanceSoFar = 0.0;
      for (size_t i = 0; i < numKPoints; ++i) {
        if (i != 0)
          distanceSoFar += distance(kpoints[i - 1], kpoints[i]);
        double x = distanceSoFar;
        bandDataStr += (QString().sprintf("%10.4f", x) + " ");
        for (size_t j = 0; j < numOrbitals; ++j) {
          // Unfortunately, these are accessed out of order here...
          bandDataStr += (QString().sprintf("%10.4f", bands[j][i]) + " ");
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

  void YaehmopExtension::plotTotalDOS() const
  {

  }

  void YaehmopExtension::plotPartialDOS() const
  {

  }

  QString YaehmopExtension::createYaehmopBandInput(bool& displayBandData) const
  {
    if (!m_molecule) {
      qDebug() << "Error in " << __FUNCTION__ << ": the molecule is not set";
      return "";
    }

    OpenBabel::OBUnitCell *cell = m_molecule->OBUnitCell();
    if (!cell) {
      QMessageBox::critical(NULL,
                        tr("Avogadro"),
                        tr("Cannot calculate band structure: no unit cell!"));
      qDebug() << "Error in " << __FUNCTION__ << ": there is no unit cell";
      return "";
    }

    // Let's get the k-point info from the user first so we don't have to run
    // through the rest of the algorithm if they cancel...
    size_t numKPoints = 0;
    QString specialKPointString;
    YaehmopBandDialog d;
    if (!d.getKPointInfo(numKPoints, specialKPointString, displayBandData))
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
    input += (QString::number(numKPoints) + "\n");
    // Num special k points
    size_t numSK = specialKPointString.split(QRegExp("[\r\n]"),
                                             QString::SkipEmptyParts).size();
    input += (QString::number(numSK) + "\n"); // num special k points
    input += specialKPointString; // Add the whole string from user input

    // And we are done!
    // qDebug() << "Input is:";
    // qDebug() << input;

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
#else
    // Otherwise, we assume yaehmop is in the same directory as avogadro
    QString program = "yaehmop";
#endif

    QStringList arguments;
    arguments << "--use_stdin_stdout";

    // If the user set a parameters file, let's use that
    if (!m_parametersFile.isEmpty())
      arguments << m_parametersFile;

    QProcess p;
    p.start(program, arguments);

    if (!p.waitForStarted()) {
      qDebug() << "Error: Yaehmop executable at" << program
               << "failed to start.";
      return false;
    }

    // Give it the input!
    p.write(input.toStdString().c_str());

    // Close the write channel
    p.closeWriteChannel();

    if (!p.waitForFinished()) {
      qDebug() << "Error: Yaehmop executable at" << program
               << "failed to finish.";
      output = p.readAll();
      qDebug() << "Output is as follows:\n" << output;
      return false;
    }

    int exitStatus = p.exitStatus();
    output = p.readAll();

    if (exitStatus == QProcess::CrashExit) {
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
      QMessageBox::critical(NULL,
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
    size_t numAtoms = atoms.size();
    // Num atoms plus 4 dummies -- dummies are for defining the lattice
    input += (QString::number(numAtoms + 4) + QString("\n"));

    // Now loop through atom positions and add them
    for (size_t i = 0; i < numAtoms; ++i) {
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
    for (size_t i = 0; i < 4; ++i) {
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
    input += "3\n"; // We are using 3 dimensions
    // Add numbers of overlaps
    for (size_t i = 0; i < 3; ++i)
      input += (QString::number(overlaps[i]) + " ");
    input += "\n";
    // If we have "4 5" here, that means the vector is defined
    // from atom 4 to atom 5. We use dummy atoms for this. The first dummy
    // atom (numAtoms + 1) is always at the origin, and the other dummy atoms
    // are at the ends of the a, b, and c axes.
    for (size_t i = 0; i < 3; ++i) {
      input += (QString::number(numAtoms + 1) + " " +
                QString::number(numAtoms + i + 2) + "\n");
    }

    return input;
  }

  void YaehmopExtension::setMolecule(Molecule *molecule)
  {
    m_molecule = molecule;
  }

  QUndoCommand* YaehmopExtension::performAction(QAction *action, GLWidget *widget)
  {
    return 0;
  }

} // end namespace Avogadro

Q_EXPORT_PLUGIN2(yaehmopextension, Avogadro::YaehmopExtensionFactory)

