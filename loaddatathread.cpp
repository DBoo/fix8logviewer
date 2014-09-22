#include "loaddatathread.h"

LoadDataThread::LoadDataThread(QMessage **qmArray, int StartPos, int EndPos,QObject *parent) :
    QThread(parent),cancelMe(false),startPos(StartPos),endPos(EndPos),qmessageArray(qmArray)
{
    if (!qmessageArray)
        return;
}
void LoadDataThread::run()
{
    for(int i=startPos;i<endPos;i++) {
        qmessageArray[i];
    }
}
void LoadDataThread::cancel()
{
    cancelMe = true;
}
