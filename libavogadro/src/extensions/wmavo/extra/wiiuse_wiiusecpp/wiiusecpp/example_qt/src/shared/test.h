#ifndef TEST_H
#define TEST_H

#include <QObject>
class MyClass : public QObject
 {
     Q_OBJECT

 public:
     MyClass(QObject *parent = 0){};
     ~MyClass(){};

 signals:
     void mySignal();

 public slots:
     void mySlot(){};
 };

#endif // TEST_H
