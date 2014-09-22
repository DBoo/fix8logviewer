#ifndef LOADDATATHREAD_H
#define LOADDATATHREAD_H

#include <QThread>
#include "messagefield.h"
#include "worksheet.h"
#include "dateTimeDelegate.h"
#include "fixHeaderView.h"
#include "fix8sharedlib.h"

#include "fixtableverticaheaderview.h"
#include "fixmimedata.h"
#include "fixtable.h"
#include "globals.h"
#include "intItem.h"
#include "messagearea.h"
#include "proxyFilter.h"
#include "tableschema.h"
#include "threadloader.h"
#include "worksheetmodel.h"
#include <QDebug>
#include <QtConcurrent>
#include <QMap>
#include <QQuickView>
#include <QtWidgets>
#include <iostream>
#include <string.h>
#include <fix8/f8includes.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>

class LoadDataThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadDataThread(QMessage **qmessageArray, int startPos, int endPos, QObject *parent = 0);
    void cancel();
protected:
    void run();
private:
    bool cancelMe;
    qint64 startPos;
    qint64 endPos;
    QMessage **qmessageArray;
};

#endif // LOADDATATHREAD_H
