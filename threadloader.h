#ifndef THREADLOADER_H
#define THREADLOADER_H

#include <QThread>
#include <QVector>
#include <QStandardItem>
#include "tableschema.h"
#include "messagefield.h"
#include "worksheetmodel.h"
class ThreadLoader : public QThread
{
    Q_OBJECT
public:
    explicit ThreadLoader(TableSchema *tableSchema,QMessageList *messageList,
                          WorkSheetModel *m,QObject *parent = 0);
protected:
    virtual void run();
signals:
    void updateGUI();
private:
    TableSchema *tableSchema;
    QMessageList *messageList;
    WorkSheetModel *model;
};

#endif // THREADLOADER_H
