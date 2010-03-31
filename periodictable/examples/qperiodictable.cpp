#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMainWindow>

#include <QtCore/QDebug>
#include <QtCore/QStringList>

#include <openbabel/mol.h>
#include <avogadro/elementtranslator.h>

#include "qperiodictable.h"

#define INSTALL_PREFIX "/usr/local"

PeriodicTableWatcher::PeriodicTableWatcher(Avogadro::PeriodicTableView *periodicTable,
                        QPlainTextEdit *elementInfo)
{
  m_periodicTable = periodicTable;
  m_elementInfo = elementInfo;
  connect(m_periodicTable, SIGNAL(elementChanged(int)),
    this, SLOT(elementChanged(int)));
}

void PeriodicTableWatcher::elementChanged(int n)
{  
  m_elementInfo->setPlainText(
    tr("Official Name") + ": " + Avogadro::ElementTranslator::name(n)
    + "\n" +tr("Atomic Number") +": " + QString::number(n)
    + "\n" +tr("Atomic Weight") +": " + QString::number(OpenBabel::etab.GetMass(n)) + " a.u."
    + "\n" +tr("Monoisotopic Weight") + ": " + QString::number(OpenBabel::isotab.GetExactMass(n)) + " a.u."
    + "\n" +tr("Covalent Radius") + ": "  + QString::number(OpenBabel::etab.GetCovalentRad(n)) + " \xC5"
    + "\n" +tr("VdW Radius") + ": "  + QString::number(OpenBabel::etab.GetVdwRad(n)) + " \xC5"
    + "\n" +tr("Pauling Electronegativity") + ": "  + QString::number(OpenBabel::etab.GetElectroNeg(n))
    + "\n" +tr("Allred-Rochow Electronegativity") + ": "  + QString::number(OpenBabel::etab.GetAllredRochowElectroNeg(n))
    + "\n" +tr("Ionization Potential") + ": "  + QString::number(OpenBabel::etab.GetIonization(n)) + " eV"
    + "\n" +tr("Electron Affinity") + ": "  + QString::number(OpenBabel::etab.GetElectronAffinity(n)) + " eV"
    );
}


int main(int argc, char **argv)
{
  QApplication app(argc, argv);

/*  // Before we do much else, load translations
  QStringList translationPaths;
  foreach (const QString &variable, QProcess::systemEnvironment()) {
    QStringList split1 = variable.split('=');
    if (split1[0] == "AVOGADRO_TRANSLATIONS") {
      foreach (const QString &path, split1[1].split(':'))
        translationPaths << path;
    }
  }
  translationPaths << QCoreApplication::applicationDirPath() + "/../share/avogadro/i18n/";
#ifdef Q_WS_MAC
  translationPaths << QString(INSTALL_PREFIX) + "/share/avogadro/i18n/";
#endif*/

  // Get the locale for translations
  QString translationCode = QLocale::system().name();

  // The QLocale::system() call on Mac doesn't reflect the default language -- only the default locale formatting
  // so we'll fine-tune the respone with QSystemLocale
  // This only applies to Qt/Mac 4.6.x and later, which added the appropriate Carbon magic to QSystemLocale.
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
#ifdef Q_WS_MAC
  QSystemLocale sysLocale;
  QLocale::Language sysLanguage = static_cast<QLocale::Language>(sysLocale.query(QSystemLocale::LanguageId, QVariant()).toInt());
  QLocale::Country sysCountry = static_cast<QLocale::Country>(sysLocale.query(QSystemLocale::CountryId, QVariant()).toInt());
  QLocale macSystemPrefsLanguage(sysLanguage, sysCountry);
  translationCode = macSystemPrefsLanguage.name();
#endif
#endif

  qDebug() << "Locale: " << translationCode;

  // Load the libavogadro translations
  QTranslator libTranslator;
  if(libTranslator.load(QString(INSTALL_PREFIX) + "/share/avogadro/i18n/libavogadro_"
     + translationCode + ".qm"))
    app.installTranslator(&libTranslator);

  // Construct Periodic Table
  Avogadro::PeriodicTableView* periodicTable = new Avogadro::PeriodicTableView;
  periodicTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QMainWindow *window = new QMainWindow();
  window->setWindowTitle("Periodic System of D.I.Mendeleyev");
  QWidget *widget = new QWidget;
  QHBoxLayout *layout = new QHBoxLayout(widget);
  widget->setLayout(layout);
  QPlainTextEdit *elementInfo = new QPlainTextEdit;
  elementInfo->setReadOnly(true);
  elementInfo->setPlainText("Click on element to get the information about it");
  elementInfo->setMaximumHeight(periodicTable->height());
  elementInfo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  layout->addWidget(periodicTable);
  layout->addWidget(elementInfo);
  window->setCentralWidget(widget);
  PeriodicTableWatcher *watcher = new PeriodicTableWatcher(periodicTable, elementInfo);
  window->show();
  app.exec();
  delete periodicTable;
  delete elementInfo;
  delete window;
  return 0;
}

