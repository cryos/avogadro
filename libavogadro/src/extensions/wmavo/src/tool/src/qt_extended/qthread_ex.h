#ifndef QTHREAD_EX_H
#define QTHREAD_EX_H

#include "warning_disable_begin.h"
#include <QThread>
#include "warning_disable_end.h"

// Normally useless. This working must be the std since ~4.6.
// And the sleep is using to avoid a cpu loading, so it can be 
// replace by yeildCurrentThread() (more efficient).
class QThread_ex : public QThread
{
protected:
  //inline void run() { exec(); } ;
public:
  inline void msleep( unsigned long ms ) { QThread::msleep(ms) ; } ;
};

#endif // QTHREAD_EX_H
