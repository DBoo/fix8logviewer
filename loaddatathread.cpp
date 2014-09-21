#include "loaddatathread.h"

LoadDataThread::LoadDataThread(QMessage **qmessageArray, int startPos, int endPos,QObject *parent) :
    QThread(parent),cancelMe(false)
{
}
void LoadDataThread::run()
{

}
void LoadDataThread::cancel()
{
    cancelMe = true;
}
