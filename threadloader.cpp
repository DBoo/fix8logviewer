#include "threadloader.h"
#include "intItem.h"
#include <QDebug>

ThreadLoader::ThreadLoader(TableSchema *ts,QMessageList *ms,
                           WorkSheetModel *m,QObject *parent) :
    QThread(parent),tableSchema(ts),messageList(ms),model(m)
{
}
void ThreadLoader::run()
{
    typedef QList<QPersistentModelIndex> QPersistentModelIndex;
    qRegisterMetaType <QPersistentModelIndex> ("QPersistentModelIndex");
    int status;
    QString name;
    QString mbName;
    QMessage   *qmessage;
    Message    *message;
    MessageBase *header;
    MessageBase *trailer;
    BaseField  *baseField;
    QBaseEntry *tableHeader;
    GroupBase  *groupBase;
    FieldTrait::FieldType ft;
    char c[60];
    int fieldID;
    int rowPos = 0;
    int colPos = 0;
    bool modifyBackgroundColor;
    model->moveToThread(this);
    QColor modBGColor; // = QColor(255,214,79,100);
    // This is a list of messages read in from file
    QListIterator <QMessage *> mIter(*messageList);
    // this is the fields user selected that they want displayed
    QListIterator <QBaseEntry *> tableHeaderIter(*(tableSchema->fieldList));
    while(mIter.hasNext()) {
        if (rowPos%500 == 0) {
            emit updateGUI();
            //qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,200);
           ;// qApp->processEvents(QEventLoop::WaitForMoreEvents,200);
        }

        qmessage = mIter.next();
        QString senderID = qmessage->senderID;

        if (messageList->senderColorMap.contains(qmessage->senderID) ) {
            modBGColor =messageList->senderColorMap.value(qmessage->senderID);
            modifyBackgroundColor = true;
        }
        else
            modifyBackgroundColor = false;
        QVariant var;
        var.setValue((void *) qmessage);
        message  = qmessage->mesg;
        header = message->Header();
        trailer = message->Trailer();
        tableHeaderIter.toFront();
        colPos = 0;
        bool found;
        while(tableHeaderIter.hasNext()) {
            found = false;
            tableHeader = tableHeaderIter.next();
            fieldID = tableHeader->ft->_fnum;
            BaseField *bf = header->get_field(fieldID);
            if (bf) {
                ft =  bf->get_underlying_type();
                memset(c,'\0',60);
                bf->print(c);
                if (FieldTrait::is_int(ft)) {
                    int ival(static_cast<Field<int, 0>*>(bf)->get());
                    //qDebug() << tableHeader->name << ", field id = " << fieldID << ", value = " << ival;
                    IntItem *intItem = new IntItem(ival);
                    //QStandardItem *intItem = new QStandardItem(QString::number(ival));

                    //intItem->setData(ival,Qt::UserRole);
                    intItem->setData(senderID,WorkSheetModel::senderIDRole);
                    intItem->setData(var);
                    if (modifyBackgroundColor)
                        intItem->setData(modBGColor, Qt::BackgroundRole);
                    model->setItem(rowPos,colPos,intItem);
                    found = true;
                }

                else if (FieldTrait::is_float(ft)) {
                    qDebug() << "WORK WITH FLOAT" << __FILE__ << __LINE__;
                    double fval(static_cast<Field<double, 0>*>(bf)->get());
                    found = true;

                }

                else if (FieldTrait::is_string(ft)) {
                    memset(c,'\0',60);
                    bf->print(c);
                    QLatin1Literal ll(c);
                    QStandardItem *strItem = new QStandardItem(QString(ll));
                    strItem->setData(senderID,WorkSheetModel::senderIDRole);
                    strItem->setData(var);
                    if (modifyBackgroundColor)
                        strItem->setData(modBGColor, Qt::BackgroundRole);
                    model->setItem(rowPos,colPos,strItem);
                    found = true;

                }
                else if (FieldTrait::is_char(ft)) {
                    QChar ch(static_cast<Field<char, 0>*>(bf)->get());
                    QString cstr = ch.decomposition();
                    QStandardItem *charItem = new QStandardItem(cstr);
                    charItem->setData(senderID,WorkSheetModel::senderIDRole);
                    charItem->setData(var);
                    if (modifyBackgroundColor)
                        charItem->setData(modBGColor, Qt::BackgroundRole);
                    model->setItem(rowPos,colPos,charItem);
                    found = true;

                }

            }
            BaseField *bfm = message->get_field(fieldID);
            if (bfm) {
                ft =  bfm->get_underlying_type();
                memset(c,'\0',60);
                bfm->print(c);
                if (FieldTrait::is_int(ft)) {
                    int ival(static_cast<Field<int, 0>*>(bfm)->get());
                    //qDebug() << tableHeader->name << ", field id = " << fieldID << ", value = " << ival;
                    IntItem *intItem = new IntItem(ival);
                    intItem->setData(senderID,WorkSheetModel::senderIDRole);
                    intItem->setData(var);
                    if (modifyBackgroundColor)
                        intItem->setData(modBGColor, Qt::BackgroundRole);
                    model->setItem(rowPos,colPos,intItem);
                    found = true;

                }
                else if (FieldTrait::is_float(ft)) {
                    qDebug() << "WORK WITH FLOAT" << __FILE__ << __LINE__;
                    double fval(static_cast<Field<double, 0>*>(bfm)->get());
                    found = true;
                }

                else if (FieldTrait::is_string(ft)) {
                    memset(c,'\0',60);
                    bfm->print(c);
                    QStandardItem *strItem = new QStandardItem(QLatin1Literal(c));
                    strItem->setData(senderID,WorkSheetModel::senderIDRole);
                    strItem->setData(var);
                    if (modifyBackgroundColor)
                        strItem->setData(modBGColor, Qt::BackgroundRole);
                    model->setItem(rowPos,colPos,strItem);
                    found = true;
                }
                else if (FieldTrait::is_char(ft)) {
                    QChar ch(static_cast<Field<char, 0>*>(bfm)->get());
                    QString cstr = ch.decomposition();
                    QStandardItem *charItem = new QStandardItem(cstr);
                    charItem->setData(senderID,WorkSheetModel::senderIDRole);
                    charItem->setData(var);
                    if (modifyBackgroundColor)
                        charItem->setData(modBGColor, Qt::BackgroundRole);
                    model->setItem(rowPos,colPos,charItem);
                    found = true;
                }
            }
            //else
            //   qWarning() << "BASE FIELD = NULL FOR HEADER" << __FILE__ << __LINE__;
            Groups groups = message->get_groups();
            std::map<unsigned short,GroupBase *>::iterator iterGrps;
            for(iterGrps = groups.begin(); iterGrps != groups.end(); iterGrps++) {
                groupBase = iterGrps->second;
                int size = groupBase->size();
                for(int i=0;i<size;i++) {
                    MessageBase *mb = groupBase->get_element(i);
                    mbName = QString::fromStdString(mb->get_msgtype());
                    BaseField *bfg = mb->get_field(fieldID);
                    if (bfg) {
                        ft =  bfg->get_underlying_type();
                        memset(c,'\0',60);
                        bfg->print(c);
                        if (FieldTrait::is_int(ft)) {
                            int ival(static_cast<Field<int, 0>*>(bfg)->get());
                            //qDebug() << tableHeader->name << ", field id = " << fieldID << ", value = " << ival;
                            IntItem *intItem = new IntItem(ival);
                            intItem->setData(senderID,WorkSheetModel::senderIDRole);
                            intItem->setData(var);
                            if (modifyBackgroundColor)
                                intItem->setData(modBGColor, Qt::BackgroundRole);
                            model->setItem(rowPos,colPos,intItem);
                            found = true;
                        }
                        else if (FieldTrait::is_float(ft)) {
                            qDebug() << "WORK WITH FLOAT" << __FILE__ << __LINE__;
                            double fval(static_cast<Field<double, 0>*>(bfg)->get());
                            found = true;
                        }
                        else if (FieldTrait::is_string(ft)) {
                            memset(c,'\0',60);
                            bfg->print(c);
                            QStandardItem *strItem = new QStandardItem(QLatin1Literal(c));
                            strItem->setData(senderID,WorkSheetModel::senderIDRole);
                            strItem->setData(var);
                            if (modifyBackgroundColor)
                                strItem->setData(modBGColor, Qt::BackgroundRole);
                            model->setItem(rowPos,colPos,strItem);
                            found = true;
                        }
                        else if (FieldTrait::is_char(ft)) {
                            QChar ch(static_cast<Field<char, 0>*>(bfm)->get());
                            QString cstr = ch.decomposition();
                            QStandardItem *charItem = new QStandardItem(cstr);
                            charItem->setData(senderID,WorkSheetModel::senderIDRole);
                            charItem->setData(var);
                            if (modifyBackgroundColor)
                                charItem->setData(modBGColor, Qt::BackgroundRole);
                            model->setItem(rowPos,colPos,charItem);
                            found = true;
                        }
                    }
                    //  else
                    //     qWarning() << "BASE FIELD = NULL FOR GROUP" << __FILE__ << __LINE__;
                    // qDebug() << "\t\tHave Message Named: " + mbName;
                }
            }
            if (trailer) {
                BaseField *bft = trailer->get_field(fieldID);
                if (bft) {
                    ft =  bft->get_underlying_type();
                    memset(c,'\0',60);
                    bft->print(c);
                    if (FieldTrait::is_int(ft)) {
                        int ival(static_cast<Field<int, 0>*>(bft)->get());
                        IntItem *intItem = new IntItem(ival);
                        intItem->setData(senderID,WorkSheetModel::senderIDRole);
                        intItem->setData(var);
                        if (modifyBackgroundColor)
                            intItem->setData(modBGColor, Qt::BackgroundRole);
                        model->setItem(rowPos,colPos,intItem);
                        found = true;
                    }
                    else if (FieldTrait::is_float(ft)) {
                        qDebug() << "WORK WITH FLOAT" << __FILE__ << __LINE__;
                        double fval(static_cast<Field<double, 0>*>(bft)->get());
                        found = true;
                    }
                    else if (FieldTrait::is_string(ft)) {
                        memset(c,'\0',60);
                        bft->print(c);
                        QStandardItem *strItem = new QStandardItem(QLatin1Literal(c));
                        strItem->setData(senderID,WorkSheetModel::senderIDRole);
                        strItem->setData(var);
                        if (modifyBackgroundColor)
                            strItem->setData(modBGColor, Qt::BackgroundRole);
                        model->setItem(rowPos,colPos,strItem);
                        found = true;
                    }
                    else if (FieldTrait::is_char(ft)) {
                        QChar ch(static_cast<Field<char, 0>*>(bft)->get());
                        QString cstr = ch.decomposition();
                        QStandardItem *charItem = new QStandardItem(cstr);
                        charItem->setData(senderID,WorkSheetModel::senderIDRole);
                        charItem->setData(var);
                        if (modifyBackgroundColor)
                            charItem->setData(modBGColor, Qt::BackgroundRole);
                        model->setItem(rowPos,colPos,charItem);
                        found = true;
                    }
                }
                //else
                // qWarning() << "BASE FIELD = NULL FOR TRAILER"  << __FILE__ << __LINE__;
            }
            if (!found) {
                //qDebug() << "**************** NOT FOUND **********************" << colPos << __FILE__ << __LINE__;
                // create a dummmy item, so color of row can be uniform across;
                QStandardItem *dummyItem = new QStandardItem("");
                dummyItem->setData(senderID,WorkSheetModel::senderIDRole);
                dummyItem->setData(var);
                if (modifyBackgroundColor)
                    dummyItem->setData(modBGColor, Qt::BackgroundRole);
                model->setItem(rowPos,colPos,dummyItem);
            }
            colPos++;
        }
        rowPos++;
    }
}



