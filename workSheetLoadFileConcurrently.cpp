//-------------------------------------------------------------------------------------------------
/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------

#include <memory>
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

using namespace FIX8;

extern void threadLoadMessage(QMessage *qm,Message *m,QLatin1String senderID,int seq,std::function<const F8MetaCntx&()> ctxFunc);

bool WorkSheet::loadFileConcurrently(QString &fileName,
                             QList <GUI::ConsoleMessage> &msgList,
                             TableSchema *ts, quint32 &returnCode)
{
    fixFileName = fileName;
    tableSchema = ts;
    QFileInfo fi(fixFileName);
    bool bstatus;
    msg_type mt;
    msg_seq_num snum;
    QString str;
    QString errorStr;
    QString name;
    //TEX::SenderCompID scID;
    QString qstr;
    QFile dataFile(fileName);
    cancelReason = OK; // clear cancel reason
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,5);
    //setUpdatesEnabled(false);
   // showLoadProcess(true);

    bstatus =  dataFile.open(QIODevice::ReadOnly);
    if (!bstatus) {
        GUI::ConsoleMessage message(tr("Failed to open file: ") + fileName);
        msgList.append(message);
        _model->clear();
        returnCode = OPEN_FAILED;
        showLoadProcess(false);

        return false;
    }
    QByteArray ba;
    int linecount=0;
    while(!dataFile.atEnd()) {
        ba = dataFile.readLine();
        linecount++;
    }
    showLoadProcess(true,linecount);
    dataFile.seek(0);
    int i=0;
    QElapsedTimer myTimer;
    int nMilliseconds;
    qint32 fileSize = dataFile.size();


    myTimer.start();
    QMap <QString, qint32> senderMap; // <sender id, numofoccurances>
    messageList = new QMessageList();
    QMessage *messageArray = new QMessage[linecount];



    char c[60];
    int goodRows = 0;
    while(!dataFile.atEnd()) {
        if (cancelLoad) {
            dataFile.close();
            if (cancelReason == TERMINATED)  // set from terminate
                returnCode = TERMINATED;
            //setUpdatesEnabled(true);
            showLoadProcess(false);

            return false;
        }
        try {
            msg_seq_num snum;
            sender_comp_id senderID;
            ba = dataFile.readLine();
            ba.truncate(ba.size()-1);
            Message *msg = Message::factory(sharedLib->ctxFunc(),ba.data());
            msg->Header()->get(snum);
            msg->Header()->get(senderID);
            memset(c,'\0',60);
            senderID.print(c);
            QLatin1String sid(c);
            //QMessage *qmessage = new QMessage(); //msg,sid,snum(),sharedLib->ctxFunc);

            // multhreading here
            QtConcurrent::run(threadLoadMessage,&messageArray[goodRows], msg,sid,snum(),sharedLib->ctxFunc);

            if (i%300 == 0) { // every 300 iterations allow gui to process events
                if (cancelLoad) {
                    showLoadProcess(false);
                    returnCode = CANCEL;
                    setUpdatesEnabled(true);
                    return false;
                }
                qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,3);
            }
            if (senderMap.contains(sid)) {
                qint32 numOfTimes = senderMap.value(sid);
                numOfTimes++;
                senderMap.insert(sid,numOfTimes);
            }
            else {
                senderMap.insert(sid,1);
            }
            //messageList->append(qmessage);
            //qDebug() << "APPEND MESSAGE:" << goodRows;
            messageList->append(&messageArray[goodRows]);
            // messageList->append(&qmessage);
            goodRows++;
        }
        catch (f8Exception&  e){
            errorStr =  "Error - Invalid data in file: " + fileName + ", on  row: " + QString::number(i);
            qWarning() << "Error - " << e.what();
            msgList.append(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
    i++;
    }
    QList<qint32> valueList = senderMap.values();
    qSort(valueList.begin(), valueList.end(),qGreater<int>());

    QListIterator <qint32> valueIter(valueList);
    QStringList keyList;
    while(valueIter.hasNext()) {
        qint32 value = valueIter.next();
        QMap<QString, qint32>::const_iterator imap = senderMap.constBegin();
        while (imap != senderMap.constEnd()) {
            if (imap.value() == value) {
                keyList.append(imap.key());
                break;
            }
            ++imap;
        }
    }

    int k = 0;
    if (senderMenu) {
        senderMenu->clear();
    }
    else
        senderMenu = new QMenu(this);
    // only support up to 6 colors for filtering by senderID
    if (valueList.count() > 1) {
        int maxItems = valueList.count();
        if (maxItems >7)
            maxItems = 7;
        for( int i=0; i<maxItems; i++) {
            QString key = keyList.at(i);
            if (i!=0) // first one is special, it shows up the most, so it has no color assigned to it
                messageList->senderColorMap.insert(key,QMessageList::senderColors[i-1]);
            QAction *action = new QAction(key,this);
            action->setCheckable(true);
            action->setChecked(true);
            senderActionGroup->addAction(action);

            senderMenu->addAction(action);
        }
    }
    showAllSendersA = new QAction("Show All",this);
    senderActionGroup->addAction(showAllSendersA);
    senderMenu->addAction(showAllSendersA);

    _model->setWorkSheet(this);
    if (!tableSchema) {
        showLoadProcess(false);
        return false;
    }
    QStringListIterator colNameIter(tableSchema->fieldNames);
    i=0;
    QString colName;
    QMap <QString, qint16> columnMap;

    while(colNameIter.hasNext()) {
        colName = colNameIter.next();
        columnMap[colName] = i;
        i++;
    }
     _model->setTableSchema(*tableSchema);
    _model->setMessageList(messageList,cancelLoad);
    connect(_model,SIGNAL(updateTable()),this,SLOT(updateTableSlot()),Qt::DirectConnection);
    fixTable->setWorkSheetModel(_model);
    sm = fixTable->selectionModel();
    fixTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    FixHeaderView *fixHeader = qobject_cast <FixHeaderView *> (fixTable->horizontalHeader());
    connect(fixHeader,SIGNAL(doPopup(int,QPoint)),
            this,SLOT(popupHeaderMenuSlot(int,const QPoint &)));
    fixHeader->setSectionResizeMode(QHeaderView::Interactive);
    fixHeader->setStretchLastSection(true);
    fixHeader->setSectionsMovable(true);
    fixHeader->setSortIndicatorShown(true);
    setTimeFormat(GUI::Globals::timeFormat);

    fixTable->setSortingEnabled(true);
    float timeOfLoad = (double)(myTimer.elapsed())/1000.0;
    //qstr = "Loading of file " + fileName + " completed. " + QString::number(_model->rowCount()) + " records in " +  QString::number(timeOfLoad,'g',3) + " seconds";
   // msgList.append(GUI::ConsoleMessage(qstr));
    qstr = fileName + " completed. " +  QString::number(_model->rowCount()) + " records";
    //statusBar()->showMessage(qstr,4500);
   // setToolTip(QString::number(_model->rowCount()) + " records");
    setUpdatesEnabled(true);
 //   showLoadProcess(false);
    returnCode = OK;
   showLoadProcess(false);

    return true;
}
