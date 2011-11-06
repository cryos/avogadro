#ifndef QTHREAD_EX_H
#define QTHREAD_EX_H

#include <QThread>

class QThread_ex : public QThread
{
protected:
    void run() { exec(); }
};

#endif // QTHREAD_EX_H
