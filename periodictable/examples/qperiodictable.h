#include <QtGui/QPlainTextEdit>
#include <avogadro/periodictableview.h>

class PeriodicTableWatcher : public QObject
{
  Q_OBJECT

  public:
    PeriodicTableWatcher(Avogadro::PeriodicTableView *periodicTable,
                         QPlainTextEdit *elementInfo);
    
  public slots:
    void elementChanged(int);

  private:
  Avogadro::PeriodicTableView *m_periodicTable;
  QPlainTextEdit *m_elementInfo;

};
