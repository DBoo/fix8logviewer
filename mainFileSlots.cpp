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
#include "globals.h"
#include "mainwindow.h"
#include "messagearea.h"
#include "fixmimedata.h"
#include "fix8sharedlib.h"
#include "globals.h"
#include "intItem.h"
#include "nodatalabel.h"
#include "worksheet.h"
#include <QDebug>
#include <QtWidgets>
#include <iostream>
#include <string.h>
#include <fix8/f8includes.hpp>
#include <fix8/message.hpp>


using namespace FIX8;
void MainWindow::createTabSlot()
{
    QString str;
    if (!sharedLib || (!sharedLib->isOK)) {
       str = "Unable to open new tab as the FIX Schema lib\nis not set for this window.";
       QMessageBox::warning(this,GUI::Globals::appName,str);
       return;
    }

    if (cursor().shape() == Qt::BusyCursor) {
        QMessageBox::information(this,GUI::Globals::appName,"This View is Currently Busy");
        return;
    }
    setCursor(Qt::BusyCursor);
    if (!fileDialog) {
        QStringList filters;
        if (fileFilter == "*.logs")
            filters << "*.log" << "*";
        else
            filters <<  "*" << "*.log" << "*.xml" << "*.data" << "*.fix";
        fileDialog = new QFileDialog(this);
        fileDialog->setWindowTitle("Open Log File For Schema \"" + sharedLib->name +"\"");
        fileDialog->setNameFilters(filters);
        fileDialog->setLabelText(QFileDialog::FileName,"Fix Log File");


        fileDialog->setFileMode(QFileDialog::ExistingFiles);
        fileDialog->setOption(QFileDialog::ReadOnly,true);
        connect(fileDialog,SIGNAL(directoryEntered(const QString &)),this,SLOT(fileDirChangedSlot(const QString &)));
        connect(fileDialog,SIGNAL(finished(int)),this,SLOT(fileSelectionFinishedSlot(int)));
        connect(fileDialog,SIGNAL(filterSelected(QString)),this,SLOT(fileFilterSelectedSlot(QString)));
        fileDialog->restoreState(fileDirState);
        fileDialog->setDirectory(lastSelectedDir);
    }
    fileDialog->show();
    unsetCursor();
}
void MainWindow::fileDirChangedSlot(const QString &newDir)
{
    QSettings settings("fix8","logviewer");
    QByteArray ba = fileDialog->saveState();
    settings.setValue("FileDirState",ba);
    settings.setValue("LastSelectedDir",newDir);
}
void MainWindow::fileFilterSelectedSlot(QString filter)
{
    QSettings settings("fix8","logviewer");
    settings.setValue("FileFilter",filter);
}

void MainWindow::fileSelectionFinishedSlot(int returnCode)
{
    //typedef std::map<std::string, unsigned> MessageCount;
    bool bstatus;
    int index;
    QStringList fileList;
    QString fileName;
    if (returnCode != QDialog::Accepted)
        return;
    if (!fileDialog) {
        qWarning() << "Error - file dialog is null, cannot get values" << __FILE__ << __LINE__;
        return;
    }
    QByteArray prevHeaderSettings;
    bool havePreviousHeader = false;
    QSettings settings("fix8","logviewer");
    settings.setValue("FileDirState",fileDialog->saveState());
    fileList = fileDialog->selectedFiles();
    QStringListIterator iter(fileList);
    if (tabW->count() > 0) {
        WorkSheet *oldWorkSheet = qobject_cast <WorkSheet *> (tabW->widget(tabW->count() -1));
        if (oldWorkSheet) {
            //prevHeaderSettings = oldWorkSheet->fixTable->horizontalHeader()->saveState();
            havePreviousHeader = true;
        }
    }
    while(iter.hasNext()) {
        fileName = iter.next();
        QFileInfo fi(fileName);
        WorkSheet *workSheet = new WorkSheet(this);
        workSheet->setSharedLib(sharedLib);
        //workSheet->setTableSchema(tableSchema);
        connect(workSheet,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
                this,SLOT(setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat)));
        connect(workSheet,SIGNAL(modelDropped(FixMimeData *)),
                this,SLOT(modelDroppedSlot(FixMimeData *)));
        connect(workSheet,SIGNAL(rowSelected(int)),
                this,SLOT(rowSelectedSlot(int)));
        workSheet->setWindowID(uuid);
        workSheet->splitter->restoreState(messageSplitterSettings);
        if (havePreviousHeader)
            workSheet->fixTable->horizontalHeader()->restoreState(prevHeaderSettings);
        QList <GUI::ConsoleMessage> messageList;
        QString str = fileName;
        if (str.length() > 36) {
            str = "..." + str.right(33);
        }
        index = tabW->addTab(workSheet,str);
        tabW->setToolTip(fileName);
        tabW->setCurrentWidget(workSheet);
        stackW->setCurrentWidget(workAreaSplitter);
        quint32 returnStatus = 0;
        //workSheet->setUpdatesEnabled(false);
        setCursor(Qt::BusyCursor);
        QElapsedTimer loadTimer;
        loadTimer.start();
        bstatus = workSheet->loadFileName(fileName,messageList,tableSchema,returnStatus);
        float timeOfLoad = (double)(loadTimer.elapsed())/1000.0;
        unsetCursor();
        if (!bstatus) {
            if (returnStatus == WorkSheet::TERMINATED) {
                str = "Loading of file: " + fileName + " terminated.";
                GUI::ConsoleMessage msg(str);
                statusBar()->showMessage(str,4000);
                messageList.append(msg);
                displayConsoleMessage(msg);

            }
            else if (returnStatus == WorkSheet::CANCEL) {
                tabW->removeTab(index);
                workSheetList.removeOne(workSheet);
                delete workSheet;
                str = "Loading of file " + fileName + " canceled.";
                GUI::ConsoleMessage msg(str);
                statusBar()->showMessage(str,4000);
                displayConsoleMessage(msg);

                messageList.append(msg);
            }
            else if (returnStatus == WorkSheet::FILE_NOT_FOUND) {
                tabW->removeTab(index);
                workSheetList.removeOne(workSheet);
                delete workSheet;
                str = "Loading of file " + fileName + " failed. File not found.";
                GUI::ConsoleMessage msg(str);
                displayConsoleMessage(msg);

                messageList.append(msg);
                statusBar()->showMessage(str,4000);
            }
            else {
                tabW->removeTab(index);
                workSheetList.removeOne(workSheet);
                delete workSheet;
                str = "Loading of file " + fileName + " failed.";
                GUI::ConsoleMessage msg(str);
                displayConsoleMessage(msg);

                messageList.append(msg);
                statusBar()->showMessage(str,4000);
            }
        }
        else {
            workSheet->setUpdatesEnabled(true);
            workSheetList.append(workSheet);
            str = "Loading of file " + fileName + " completed. " + QString::number(workSheet->getNumOfRecords()) + " records in " +  QString::number(timeOfLoad,'g',3) + " seconds";
            qDebug() << str << __FILE__ << __LINE__;
            GUI::ConsoleMessage msg(str,GUI::ConsoleMessage::InfoMsg);
             displayConsoleMessage(msg);
            statusBar()->showMessage(str,4000);
            filterSenderMenuA->setMenu(workSheet->getSenderMenu());
            SearchFunction sf = workSheet->getSearchFunction();
            setSearchFunction(sf);
        }
        // display error messages associated with each worksheet
        if (messageList.count() > 0) {
            QListIterator <GUI::ConsoleMessage> messageIter(messageList);
            while(messageIter.hasNext()) {
                GUI::ConsoleMessage message = messageIter.next();
                displayConsoleMessage(message);
            }
        }
    }
    if (tabW->count() > 0) {
        stackW->setCurrentWidget(workAreaSplitter);
        copyTabA->setEnabled(true);
        showMessageA->setEnabled(true);
        if (tabW->count() > 1)
            tabW->setCurrentIndex(index);
    }
    else {
        stackW->setCurrentWidget(noDataL);
        copyTabA->setEnabled(false);
        showMessageA->setEnabled(false);
    }
}
void MainWindow::loadFile(QString &fileName)
{
    bool bstatus;
    int index;

    QByteArray prevHeaderSettings;
    bool havePreviousHeader = false;

    if (tabW->count() > 0) {
        WorkSheet *oldWorkSheet = qobject_cast <WorkSheet *> (tabW->widget(tabW->count() -1));
        if (oldWorkSheet) {
            //prevHeaderSettings = oldWorkSheet->fixTable->horizontalHeader()->saveState();
            havePreviousHeader = true;
        }
    }
    WorkSheet *workSheet = new WorkSheet(this);
     workSheet->setSharedLib(sharedLib);
   // workSheet->setTableSchema(tableSchema);
    connect(workSheet,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
            this,SLOT(setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat)));
    connect(workSheet,SIGNAL(modelDropped(FixMimeData *)),
            this,SLOT(modelDroppedSlot(FixMimeData *)));
    connect(workSheet,SIGNAL(rowSelected(int)),
            this,SLOT(rowSelectedSlot(int)));
    workSheet->setWindowID(uuid);
    workSheet->splitter->restoreState(messageSplitterSettings);
    if (havePreviousHeader)
        workSheet->fixTable->horizontalHeader()->restoreState(prevHeaderSettings);
    QList <GUI::ConsoleMessage> messageList;
    QString str = fileName;
    if (str.length() > 36) {
        str = "..." + str.right(33);
    }
    index = tabW->addTab(workSheet,str);
    tabW->setToolTip(fileName);
    tabW->setCurrentWidget(workSheet);
    stackW->setCurrentWidget(workAreaSplitter);
    quint32 returnStatus = 0;
    //workSheet->setUpdatesEnabled(false);
    setCursor(Qt::BusyCursor);

    bstatus = workSheet->loadFileName(fileName,messageList,tableSchema,returnStatus);
    unsetCursor();
    if (!bstatus) {
        if (returnStatus == WorkSheet::TERMINATED) {
            str = "Loading of file: " + fileName + " terminated.";
            GUI::ConsoleMessage msg(str);
            statusBar()->showMessage(str,3000);
            messageList.append(msg);
        }
        else if (returnStatus == WorkSheet::CANCEL) {
            tabW->removeTab(index);
            workSheetList.removeOne(workSheet);
            delete workSheet;
            str = "Loading of file " + fileName + " canceled.";
            GUI::ConsoleMessage msg(str);
            statusBar()->showMessage(str,3000);
            messageList.append(msg);
        }
        else if (returnStatus == WorkSheet::FILE_NOT_FOUND) {
            tabW->removeTab(index);
            workSheetList.removeOne(workSheet);
            delete workSheet;
            str = "Loading of file " + fileName + " failed. File not found.";
            GUI::ConsoleMessage msg(str);
            messageList.append(msg);
            statusBar()->showMessage(str,3000);
        }
        else {
            tabW->removeTab(index);
            workSheetList.removeOne(workSheet);
            delete workSheet;
            str = "Loading of file " + fileName + " failed.";
            GUI::ConsoleMessage msg(str);
            messageList.append(msg);
            statusBar()->showMessage(str,3000);
        }
    }
    else {
        workSheet->setUpdatesEnabled(true);
        workSheetList.append(workSheet);
        str = "Loaded " + fileName + " completed";
        statusBar()->showMessage(str,2000);
        filterSenderMenuA->setMenu(workSheet->getSenderMenu());
        SearchFunction sf = workSheet->getSearchFunction();
        setSearchFunction(sf);
    }
    // display error messages associated with each worksheet
    if (messageList.count() > 0) {
        QListIterator <GUI::ConsoleMessage> messageIter(messageList);
        while(messageIter.hasNext()) {
            GUI::ConsoleMessage message = messageIter.next();
            displayConsoleMessage(message);
        }
    }

    if (tabW->count() > 0) {
        stackW->setCurrentWidget(workAreaSplitter);
        copyTabA->setEnabled(true);
        showMessageA->setEnabled(true);
        if (tabW->count() > 1)
            tabW->setCurrentIndex(index);
    }
    else {
        stackW->setCurrentWidget(noDataL);
        copyTabA->setEnabled(false);
        showMessageA->setEnabled(false);
    }
}

void MainWindow::displayConsoleMessage(GUI::ConsoleMessage message)
{
    QString str;
    QString timeStr = QTime::currentTime().toString() +
            " - " + message.msg;
    switch (message.messageType) {
    case GUI::ConsoleMessage::ErrorMsg:
        str = "<FONT COLOR=\"red\">" + timeStr + "</FONT>";
        break;
    case GUI::ConsoleMessage::WarningMsg:
        str = "<FONT COLOR=\"gold\">" + timeStr + "</FONT>";
        break;
    default:
        str = "<FONT COLOR=\"white\">" + timeStr + "</FONT>";
    }
    consoleArea->append(str);
}
void MainWindow::tabCloseRequestSlot(int tabPosition)
{
    WorkSheet *worksheet =  qobject_cast <WorkSheet *> (tabW->widget(tabPosition));
    setCursor(Qt::BusyCursor);
    if (worksheet) {

        int index = tabW->currentIndex();
        if(index == tabPosition) {
            filterSenderMenuA->setMenu(0);
        }
        worksheet->terminate(); // call in case worksheet is being loaded
    }
    tabW->removeTab(tabPosition);
    if (tabW->count() > 0) {
        stackW->setCurrentWidget(workAreaSplitter);
        WorkSheet *currentWorkSheet = qobject_cast<WorkSheet *> (tabW->currentWidget());
        if (currentWorkSheet) {
            filterSenderMenuA->setMenu(currentWorkSheet->getSenderMenu());
            SearchFunction sf = currentWorkSheet->getSearchFunction();
            setSearchFunction(sf);
        }
        copyTabA->setEnabled(true);
        showMessageA->setEnabled(true);
    }
    else {
        stackW->setCurrentWidget(noDataL);
        copyTabA->setEnabled(false);
        showMessageA->setEnabled(false);
    }
    if (worksheet) {
        workSheetList.removeOne(worksheet);
        worksheet->deleteLater();
    }
    unsetCursor();
}
void MainWindow::closeSlot()
{
    if (cursor().shape() == Qt::BusyCursor) {
        QMessageBox::information(this,"FIX8Log","This View is Currently Busy");
        return;
    }
    setCursor(Qt::BusyCursor);
    writeSettings();
    emit deleteWindow(this);
    unsetCursor();
}
void MainWindow::copyWindowSlot()
{
    if (cursor().shape() == Qt::BusyCursor) {
        QMessageBox::information(this,"FIX8Log","This View is Currently Busy");
        return;
    }
    emit copyWindow(this);
}
void MainWindow::copyTabSlot()
{
    WorkSheet *workSheet;
    WorkSheet *newWorkSheet;
    int currentTabCount = tabW->count();
    if (currentTabCount < 1) {
        qWarning() << "No tabs to copy" << __FILE__ << __LINE__;
        return;
    }
    // crazy but use curor style to see if app is busy
    if (cursor().shape() == Qt::BusyCursor) {
        QMessageBox::information(this,"FIX8Log","This View is Currently Busy");
        return;
    }
    setCursor(Qt::BusyCursor);
    workSheet = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!workSheet) {
        qWarning() << "No work sheet to copy" << __FILE__ << __LINE__;
        unsetCursor();
        return;
    }
    WorkSheetData wsd = workSheet->getWorksheetData();
    //currentItemIter =  fileNameModelMap.find(wsd.fileName);
    addWorkSheet(wsd);
    if (currentTabCount < tabW->count()) // terminated or problem loading it
        setCurrentTabAndSelectedRow(tabW->count()-1,2);

    unsetCursor();
}
void MainWindow::exportSlot(QAction *action)
{

    WorkSheet *ws  = qobject_cast <WorkSheet *> (tabW->currentWidget());
    if (!ws) {
        QMessageBox::warning(this,"Export File", "Export Failed - no file found");
        qWarning() << "Filter Failed, work sheet is null" << __FILE__ << __LINE__;
        return;
    }
    QFileInfo fi(ws->fixFileName);
    QString title;
    QStringList filters;
    QString fileName;
    if (action == exportCSVA) {
        title = "Export " + fi.fileName() + "As CSV File";
         filters << "*.csv";
    }
    else {
        title = "Export " + fi.fileName() + "As XLSX File";
        filters << "*.xlsx";
    }
    QFileDialog *fileDialog = new QFileDialog(this,title);
    QSettings settings("fix8","logviewer");

    QString exportDir = settings.value("ExportDir",QDir::homePath()).toString();
    QFileDialog::ViewMode viewMode =
                (QFileDialog::ViewMode)settings.value("ExportViewMode",QFileDialog::List).toInt();

    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setDirectory(exportDir);
    fileDialog->setViewMode(viewMode);
    ;
    fileDialog->setNameFilters(filters);
    int status = fileDialog->exec();
    exportDir = fileDialog->directory().path();
    viewMode = fileDialog->viewMode();
    settings.setValue("ExportDir",exportDir);
    settings.setValue("ExportViewMode",viewMode);
    QStringList fileNames = fileDialog->selectedFiles();
    fileDialog->deleteLater();
    if (status == QDialog::Accepted) {
            fileName  = fileNames.first();
            if (action == exportCSVA)
                exportAsCSV(fileName,ws);
            else
                exportAsXLSXA(fileName,ws);

    }
}
