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
#include "comboboxlineedit.h"
#include "editHighLighter.h"
#include "fix8sharedlib.h"
#include "fixmimedata.h"
#include "fixtoolbar.h"
#include "mainwindow.h"
#include "nodatalabel.h"
#include "worksheet.h"
#include "worksheetmodel.h"
#include "globals.h"
#include "lineedit.h"
#include "tableschema.h"
#include <QQuickView>
#include <QtWidgets>
#include <QStandardItemModel>


MainWindow::MainWindow(Database *db,bool showLoading)
    : QMainWindow(0),schemaActionGroup(0),fileDialog(0),qmlObject(0),
      windowDataID(-1),loadingActive(showLoading),tableSchema(0),haveFilterFunction(false),
        haveSearchFunction(false),database(db),linkSearchOn(false),fieldUsePairList(0),sharedLib(0),
        defaultTableSchema(0),schemaList(0),filterRunning(false),cancelFilter(false)
{
    buildMainWindow();
    if (loadingActive) {
        stackW->setCurrentIndex(ShowProgress);
    }
    copyTabA->setEnabled(false); // not enabled when no intital tab
    readSettings();
}

MainWindow::MainWindow(MainWindow &mw,Database *db,bool copyAll)
    : QMainWindow(0),schemaActionGroup(0),fileDialog(0),qmlObject(0),
      windowDataID(-1),loadingActive(false), haveFilterFunction(false),haveSearchFunction(false),database(db),linkSearchOn(false),fieldUsePairList(0),
    sharedLib(0),defaultTableSchema(0),schemaList(0)
{
    buildMainWindow();
    setAcceptDrops(true);
    QRect rect = mw.geometry();
    TableSchema *ts = mw.getTableSchema();
    setTableSchema(ts);
    setGeometry(rect);
    int x = mw.x();
    int y = mw.y();
    sharedLib = mw.getSharedLibrary();
    fieldUsePairList = mw.getFieldUsePair();
    restoreState(mw.saveState());
    if (copyAll) {
        for (int i=0;i<mw.tabW->count();i++) {
            WorkSheet *oldWorkSheet = qobject_cast <WorkSheet *> (mw.tabW->widget(i));
            QByteArray ba = oldWorkSheet->splitter->saveState();
            WorkSheet *newWorkSheet = new WorkSheet(this);
            newWorkSheet->setSharedLib(sharedLib);
            qDebug() << "REDO NEW WINDOW " << __FILE__ << __LINE__;
            newWorkSheet->copyFrom(*oldWorkSheet);
            newWorkSheet->setWindowID(uuid);
            workSheetList.append(newWorkSheet);
            connect(newWorkSheet,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
                    this,SLOT(setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat)));
            connect(newWorkSheet,SIGNAL(modelDropped(FixMimeData*)),
                    this,SLOT(modelDroppedSlot(FixMimeData*)));
            connect(newWorkSheet,SIGNAL(sendMessage(GUI::ConsoleMessage)),
                    this,SLOT(displayMessageSlot(GUI::ConsoleMessage)));
            connect(newWorkSheet,SIGNAL(terminateCopy(WorkSheet*)),
                    this,SLOT(terminatedWorkSheetCopySlot(WorkSheet*)));
            connect(newWorkSheet,SIGNAL(rowSelected(int)),
                    this,SLOT(rowSelectedSlot(int)));
            connect(newWorkSheet,SIGNAL(doPopup(const QModelIndex &, const QPoint &)),
                    this,SLOT(doPopupSlot(const QModelIndex &, const QPoint &)));
            newWorkSheet->splitter->restoreState(ba);
            QString str = mw.tabW->tabText(i);
            if (str.length() > 36) {
                str = "..." + str.right(33);
            }
            int index = tabW->addTab(newWorkSheet,str);
        }

        if (tabW->count() > 0)
            stackW->setCurrentWidget(workAreaSplitter);
        else
            stackW->setCurrentWidget(noDataL);
    }
    readSettings();
    setFont(mw.font());

    move(x+100,y+90); // offset from window copied
    linkSearchOn = mw.linkSearchOn;
    //linkSearchA->setChecked(linkSearchOn);
    searchFunction = mw.searchFunction;
    if (mainMenuBar) {
        mainMenuBar->setStyleSheet(mw.mainMenuBar->styleSheet());
        fileMenu->setStyleSheet(menuStyle);
        windowMenu->setStyleSheet(menuStyle);
        optionMenu->setStyleSheet(menuStyle);
        schemaMenu->setStyleSheet(menuStyle);
        helpMenu->setStyleSheet(menuStyle);
    }
}
void MainWindow::setLoading(bool bstatus)
{
    loadingActive = bstatus;
    if (bstatus) {
        stackW->setCurrentIndex(ShowProgress);
    }
    else {
        if (tabW->count() > 0) {
            stackW->setCurrentIndex(ShowTab);
        }
        else {
            stackW->setCurrentIndex(ShowNoDataLabel);
        }
    }
}
void MainWindow::setLoadMessage(QString str)
{
    QVariant returnedValue;
    if (!qmlObject) {
        qWarning() << "Failed to set load message, qmlObject = 0"
                   << __FILE__ << __LINE__;
        return;
    }
    QMetaObject::invokeMethod (qmlObject, "setMessage", Q_RETURN_ARG(QVariant, returnedValue),  Q_ARG(QVariant,str));
}
void MainWindow::buildMainWindow()
{
    setAutoFillBackground(true);

    setWindowIcon(QIcon(":/images/64x64/logviewer.png"));
    setAcceptDrops(true);
    uuid = QUuid::createUuid();
    setAnimated(true);
    mainMenuBar = menuBar();
    QPalette pal = mainMenuBar->palette();
    menuBG =  pal.color(QPalette::Window);
    menuFG =  pal.color(QPalette::WindowText);
    //Save this string as setStyle on menubar changes colors of menus, this is only way to get around it
    menuStyle = QString("background-color:" + menuBG.name() + QString("; color:")
                        + menuFG.name()) + QString(";");
    mainMenuBar->setAutoFillBackground(true);
    fileMenu = mainMenuBar->addMenu(tr("&File"));
    windowMenu = mainMenuBar->addMenu(tr("&Window"));
    optionMenu = mainMenuBar->addMenu(tr("&Option"));
    schemaMenu = mainMenuBar->addMenu("&Schema");
    helpMenu = mainMenuBar->addMenu("&Help");
    mainToolBar = new QToolBar("Main Toolbar",this);
    mainToolBar->setObjectName("MainToolBar");

    filterToolBar = new QToolBar("Filter",this);
    connect(filterToolBar,SIGNAL(visibilityChanged(bool)),
            this,SLOT(filterToolbarVisibleSlot(bool)));
    filterToolBar->setObjectName("FilterToolBar");
    filterToolBar->setAutoFillBackground(true);
    filterToolBarA =   filterToolBar->toggleViewAction();
    QIcon filterIcon;
    filterIcon.addPixmap(QPixmap(":/images/svg/filterOn.svg"),QIcon::Normal,QIcon::On);
    filterIcon.addPixmap(QPixmap(":/images/svg/filterOff.svg"),QIcon::Normal,QIcon::Off);
    filterToolBarA->setIcon(filterIcon);
    filterToolBarA->setToolTip("Filter Messages");

    filterToolBarA->setWhatsThis("Filter out messages");
    filterArea = new QWidget(this);
    QHBoxLayout *filterBox = new QHBoxLayout();
    filterBox->setMargin(0);
    filterArea->setLayout(filterBox);
    filterL = new QLabel(filterArea);
    filterL->setText(tr("Filter:"));
    filterLineEdit = new LineEdit(filterArea);
    connect(filterLineEdit,SIGNAL(textChanged()),this,SLOT(filterTextChangedSlot()));
    connect(filterLineEdit,SIGNAL(returnPressed()),this,SLOT(filterReturnSlot()));
    filterLineEdit->setWhatsThis("Filter critera. Can be any Javascript\nEg: MsgType=='8' && AvgPx > 10.0");
    filterBox->addWidget(filterL,0);
    filterBox->addWidget(filterLineEdit,1);
    filterButtonGroup = new QButtonGroup(this);
    excludeFilterB = new QRadioButton("Exclude",this);
    excludeFilterB->setToolTip("Apply Filter to \'exlude\' by rule");
    includeFilterB = new QRadioButton("Include",this);
    includeFilterB->setToolTip("Apply Filter to \'include' by rule");
    offFilterB  = new QRadioButton("Off",this);
    offFilterB->setToolTip("Turn off filter");

    filterButtonGroup->setExclusive(true);
    filterButtonGroup->addButton(excludeFilterB,WorkSheetData::Exclusive);
    filterButtonGroup->addButton(includeFilterB,WorkSheetData::Inclusive);
    filterButtonGroup->addButton(offFilterB,WorkSheetData::Off);
    offFilterB->setChecked(true);
    connect(filterButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(filterModeChangedSlot(int)));

    filterBox->addWidget(excludeFilterB,0);
    filterBox->addWidget(includeFilterB,0);
    filterBox->addWidget(offFilterB,0);

    QWidget *filterSelectArea = new QWidget(this);
    QHBoxLayout *filterSelectBox = new QHBoxLayout();
    filterSelectArea->setLayout(filterSelectBox);
    filterSelectBox->setMargin(0);
    filterSelectL = new QLabel("Select:");
    filterSelectCB = new QComboBox(this);
    connect(filterSelectCB,SIGNAL(currentIndexChanged(int)),this,SLOT(filterFunctionSelectedSlot(int)));
    filterSelectLineEdit = new ComboBoxLineEdit(this);
    filterSelectCB->setLineEdit(filterSelectLineEdit);
    QFontMetrics fm2(filterSelectCB->font());
    filterSelectCB->setMaximumWidth(fm2.maxWidth()*20);
    //filterSelectModel = filterSelectCB->model();
    filterSelectCB->setEditable(false);
    filterSelectCB->setToolTip("Alias list of filter functions");
    filterSelectCB->setWhatsThis("Select filter function by its alias.  Use \"Search Edit\" to edit this list");

    filterSelectBox->addWidget(filterSelectL,0);
    filterSelectBox->addWidget(filterSelectCB,1);

    editFilterA= new QAction("&Filter Editor",this);
    editFilterA->setIconText("Edit");
    editFilterA->setToolTip(tr("Edit Filter"));
    editFilterA->setWhatsThis(tr("Allows you to filter out messages"));
    editFilterA->setIcon((QIcon(":/images/svg/filterEdit.svg")));
    connect(editFilterA,SIGNAL(triggered()),this,SIGNAL(showFilterDialog()));

    QIcon saveIcon;
    saveIcon.addPixmap(QPixmap(":/images/svg/saveEnabled.svg"),QIcon::Normal,QIcon::On);
    saveIcon.addPixmap(QPixmap(":/images/svg/saveDisabled.svg"),QIcon::Normal,QIcon::Off);
    saveFilterFuncA  = new QAction("Save",this);

    saveFilterFuncA->setIcon(saveIcon);
    saveFilterFuncA->setToolTip(tr("Save this filter criteria"));
    saveFilterFuncA->setWhatsThis(tr("Save filter string to database so it can be reused latter"));
    connect(saveFilterFuncA,SIGNAL(triggered()),this,SLOT(saveFilterStringSlot()));
   // searchToolBar->addAction(linkSearchA);
    //searchToolBar->addWidget(searchLV);
    filterToolBar->addWidget(filterArea);
    filterToolBar->addAction(saveFilterFuncA);
    filterToolBar->addAction(editFilterA);

    filterProgressBar = new QProgressBar(filterToolBar);
    filterProgressBar->setMinimum(0);
    filterProgressBar->setMaximum(100);
    filterProgressBar->setToolTip("Filtering Completed");
    filterSelectBox->addSpacing(32);
    filterSelectBox->addWidget(filterProgressBar);
    filterToolBar->addWidget(filterSelectArea);
    cancelFilterA = new QAction("Cancel Filter",this);
    connect(cancelFilterA,SIGNAL(triggered()),this,SLOT(cancelFilterSlot()));
    QIcon cancelFilterIcon;
    cancelFilterIcon.addPixmap(QPixmap(":/images/svg/cancel.svg"),QIcon::Normal,QIcon::On);
    cancelFilterIcon.addPixmap(QPixmap(":/images/svg/cancel.svg"),QIcon::Normal,QIcon::Off);
    cancelFilterA->setIcon(cancelFilterIcon);
    filterToolBar->addAction(cancelFilterA);
    QWidget* empty = new QWidget();
    empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    filterToolBar->addWidget(empty);
    searchToolBar = new FixToolBar("Search",this);
    connect(searchToolBar,SIGNAL(orientationChanged(Qt::Orientation)),
            this,SLOT(toolbarOrientationChangedSlot(Qt::Orientation)));
    connect(searchToolBar,SIGNAL(visibilityChanged(bool)),
            this,SLOT(searchToolbarVisibleSlot(bool)));
    searchToolBar->setObjectName("SearchToolBar");
    searchToolBarA = searchToolBar->toggleViewAction();
    searchToolBarA->setToolTip("Search For Records");
    QIcon searchIcon;
    searchIcon.addPixmap(QPixmap(":/images/svg/searchOn.svg"),QIcon::Normal,QIcon::On);
    searchIcon.addPixmap(QPixmap(":/images/svg/searchOff.svg"),QIcon::Normal,QIcon::Off);
    searchToolBarA->setIcon(searchIcon);
    hideToolBarA = mainToolBar->toggleViewAction();

    fontActionGroup = new QActionGroup(this);
    fontIncreaseA = new QAction(QIcon(":/images/svg/increasefont.svg"),"Larger Font",this);
    fontDecreaseA = new QAction(QIcon(":/images/svg/decreasefont.svg"),"Smaller Font",this);
    fontRegularA  = new QAction(QIcon(":/images/svg/font.svg"),"Regular Font",this);
    fontActionGroup->addAction(fontIncreaseA);
    fontActionGroup->addAction(fontDecreaseA);
    fontActionGroup->addAction(fontRegularA);
    connect(fontActionGroup,SIGNAL(triggered(QAction*)),this,SLOT(setFontSlot(QAction*)));

    mainToolBar->setMovable(true);
    filterToolBar->setMovable(true);
    searchToolBar->setMovable(true);
    searchToolBar->setAllowedAreas(Qt::TopToolBarArea|Qt::BottomToolBarArea);
    addToolBar(Qt::TopToolBarArea,mainToolBar);
    addToolBar(Qt::TopToolBarArea,searchToolBar);
    addToolBar(Qt::TopToolBarArea,filterToolBar);
    if (GUI::Globals::isFirstTime) {
        insertToolBarBreak(searchToolBar);
        insertToolBarBreak(filterToolBar);
    }
    autoSaveA = new  QAction(tr("&Auto Save"),this);
    QIcon autoIcon;
    autoIcon.addPixmap(QPixmap(":/images/svg/saveOn.svg"),QIcon::Normal,QIcon::On);
    autoIcon.addPixmap(QPixmap(":/images/svg/saveOff.svg"),QIcon::Normal,QIcon::Off);
    autoSaveA->setIcon(autoIcon);
    autoSaveA->setToolTip(tr("Automatically Save Session For Next Use"));
    autoSaveA->setWhatsThis(tr("When  turned all windows and tabs will be saved and restored upon next start of application"));
    autoSaveA->setCheckable(true);
    connect(autoSaveA,SIGNAL(triggered(bool)),this,SLOT(autoSaveOnSlot(bool)));
    closeA = new QAction(tr("&Close Window"),this);
    closeA->setIcon(QIcon(":/images/32x32/application-exit.png"));
    closeA->setToolTip(tr("Close This Window"));
    closeA->setWhatsThis(tr("Closes only this window, if last window open then it will exit application"));
    connect(closeA,SIGNAL(triggered()),this,SLOT(closeSlot()));

    quitA = new QAction(tr("&Quit"),this);
    quitA->setIcon(QIcon(":/images/32x32/exit.svg"));
    quitA->setToolTip(tr("Exit Application"));
    quitA->setWhatsThis(tr("Exit application, clsoing all windows"));
    connect(quitA,SIGNAL(triggered()),this,SLOT(quitSlot()));

    newTabA = new QAction(tr("New Tab"),this);
    newTabA->setIcon((QIcon(":/images/svg/newspreadsheet.svg")));
    newTabA->setToolTip(tr("Create A New Empty Tab"));
    newTabA->setWhatsThis(tr("Create a new tab in this window"));

    copyTabA = new QAction(tr("Copy Tab"),this);
    copyTabA->setIcon((QIcon(":/images/svg/spreadsheetCopy.svg")));
    copyTabA->setToolTip(tr("Create New Tab From Current Tab"));
    connect(copyTabA,SIGNAL(triggered()),this,SLOT(copyTabSlot()));

    cutTabA = new QAction(tr("Cut Tab"),this);
    cutTabA->setIcon((QIcon(":/images/svg/spreadsheetCopy.svg")));
    cutTabA->setToolTip(tr("Cut Tab For Pasting To Other Window"));
    newWindowA = new QAction("New &Window",this);
    newWindowA->setIcon((QIcon(":/images/32x32/newwindow.svg")));
    newWindowA->setToolTip(tr("Open New Window"));
    newWindowA->setWhatsThis(tr("Open a new empty window"));
    windowNameA = new QAction("Change Window Name",this);
    windowNameA->setToolTip("Rename this window");
    connect(windowNameA,SIGNAL(triggered()),this,SLOT(setWindowNameSlot()));

    copyWindowA = new QAction("&Copy Window",this);
    copyWindowA->setIcon((QIcon(":/images/32x32/copywindow.svg")));
    copyWindowA->setToolTip(tr("Copy Window"));
    copyWindowA->setWhatsThis(tr("Create a new window exactly like this one"));
    connect(copyWindowA,SIGNAL(triggered()),this,SLOT(copyWindowSlot()));
    editSchemaA= new QAction("&Schema Editor",this);
    editSchemaA->setIconText("Edit");
    editSchemaA->setToolTip(tr("Edit table columns"));
    editSchemaA->setWhatsThis(tr("Allows you to select what fields should be displayed as columns in table"));
    editSchemaA->setIcon((QIcon(":/images/svg/editSchema.svg")));
    connect(editSchemaA,SIGNAL(triggered()),this,SLOT(editSchemaSlot()));
    showMessageA = new QAction(tr("Show/Hide Msgs"),this);
    showMessageA->setToolTip(tr("Show/Hide Message Area"));
    showMessageA->setWhatsThis(tr("Message area displays errors, warnings and status"));
    showMessageA->setCheckable(true);
    connect(showMessageA,SIGNAL(triggered(bool)),this,SLOT(showMessageArea(bool)));
    QIcon showIcon;
    showIcon.addPixmap(QPixmap(":/images/svg/showMessageArea.svg"),QIcon::Normal,QIcon::Off);
    showIcon.addPixmap(QPixmap(":/images/svg/hideMessageArea.svg"),QIcon::Normal,QIcon::On);
    showMessageA->setIcon(showIcon);



    filterSenderMenuA = new QAction("Sender",this);
    filterSenderMenuA->setIcon(QIcon(":/images/svg/filterSender.svg"));
    filterSenderMenuA->setToolTip("Filter Out Messages By SenderID");
    filterSenderMenuA->setWhatsThis("Allows you to filter out messages by SenderID");
    searchActionGroup = new QActionGroup(this);
    connect(searchActionGroup,SIGNAL(triggered(QAction*)),
            this,SLOT(searchActionSlot(QAction*)));
    /* set above for saveFilterFuncA
    QIcon saveIcon;
    saveIcon.addPixmap(QPixmap(":/images/svg/saveEnabled.svg"),QIcon::Normal,QIcon::On);
    saveIcon.addPixmap(QPixmap(":/images/svg/saveDisabled.svg"),QIcon::Normal,QIcon::Off);
    */
 saveSearchFuncA  = new QAction("Save",this);

    saveSearchFuncA->setIcon(saveIcon);
    saveSearchFuncA->setToolTip(tr("Save this search criteria"));
    saveSearchFuncA->setWhatsThis(tr("Save search string to database so it can be reused latter"));
    connect(saveSearchFuncA,SIGNAL(triggered()),this,SLOT(saveSearchStringSlot()));
    searchBackA  = new QAction("Previous",this);
    searchBackA->setToolTip("Search Back");
    searchBackA->setWhatsThis("Go to preceeding message that matches search critera");
    searchBackA->setIcon(QIcon(":/images/svg/go-previous-symbolic.svg"));
    searchBeginA = new QAction("First",this);
    searchBeginA->setToolTip("Go to first search entry");
    searchBeginA->setWhatsThis("Go to firs message that matches search requirement");
    searchBeginA->setIcon(QIcon(":/images/svg/go-first-symbolic.svg"));
    searchEndA   = new QAction("Last",this);
    searchEndA->setToolTip("Go to last search entry");
    searchEndA->setWhatsThis("Go to last message that matches search requirement");
    searchEndA->setIcon(QIcon(":/images/svg/go-last-symbolic.svg"));
    searchNextA  = new QAction("Next",this);
    searchNextA->setToolTip("Search Forward");
    searchNextA->setWhatsThis("Go to next message that matches search critera");
    searchNextA->setIcon(QIcon(":/images/svg/go-next-symbolic.svg"));
    searchActionGroup->addAction(searchBackA);
    searchActionGroup->addAction(searchBeginA);
    searchActionGroup->addAction(searchEndA);
    searchActionGroup->addAction(searchNextA);

    searchEditA  = new QAction(tr("Edit"),this);
    searchEditA->setToolTip("Edit search functions");
    searchEditA->setWhatsThis("Allows you to add remove,edit and provide aliases for search functions");
    connect(searchEditA,SIGNAL(triggered()),this,SIGNAL(showSearchDialog()));
    //searchEditA->setIcon(QIcon(":/images/svg/searchEdit.svg"));
    QSize stoolbar = searchToolBar->iconSize();
    QPixmap searchEditPM(":/images/svg/searchEdit.svg");
    int ht = stoolbar.height()*.66;
    searchEditA->setIcon(searchEditPM.scaledToHeight(ht));
    searchLV = new QLabel(searchToolBar);// only show when toobar is vertial
    searchArea = new QWidget(this);
    QHBoxLayout *searchBox = new QHBoxLayout();
    searchBox->setMargin(0);
    searchArea->setLayout(searchBox);
    QIcon linkIcon;
    linkIcon.addPixmap(QPixmap(":/images/128x128/broken_link-128.png"),QIcon::Normal,QIcon::Off);
    linkIcon.addPixmap(QPixmap(":/images/128x128/chainlink_128x128-32.png"),QIcon::Normal,QIcon::On);
    /*
    linkSearchA = new QAction("Search All",this);
    linkSearchA->setCheckable(true);
    linkSearchA->setToolTip("Search all tabs or just current tab");
    linkSearchA->setWhatsThis("When on, search applies to all tabs of this window");
    linkSearchA->setIcon(linkIcon);
    connect(linkSearchA,SIGNAL(toggled(bool)),SLOT(linkSearchSlot(bool)));
    */
    searchL = new QLabel(searchArea);
    searchL->setText(tr("Search:"));
    searchLineEdit = new LineEdit(searchArea);
    searchLineEdit->setWhatsThis("Search critera. Can be any Javascript\nEg: MsgType=='8' && AvgPx > 10.0");

    QWidget *searchSelectArea = new QWidget(this);
    QHBoxLayout *searchSelectBox = new QHBoxLayout();
    searchSelectArea->setLayout(searchSelectBox);
    searchSelectBox->setMargin(0);
    searchSelectL = new QLabel("Select:");
    searchSelectCB = new QComboBox(this);
    connect(searchSelectCB,SIGNAL(currentIndexChanged(int)),this,SLOT(searchFunctionSelectedSlot(int)));
    searchSelectLineEdit = new ComboBoxLineEdit(this);
    searchSelectCB->setLineEdit(searchSelectLineEdit);
    QFontMetrics fm(searchSelectCB->font());
    searchSelectCB->setMaximumWidth(fm.maxWidth()*21);
    searchSelectModel = searchSelectCB->model();
    searchSelectCB->setEditable(false);
    searchSelectCB->setToolTip("Alias list of search functions");
    searchSelectCB->setWhatsThis("Select search function by its alias.  Use \"Search Edit\" to edit this list");

    searchSelectBox->addWidget(searchSelectL,0);
    searchSelectBox->addWidget(searchSelectCB,1);
    searchSelectBox->addStretch(1);

    searchCompleter = new QCompleter(this);

    searchCompleter->setCompletionMode(QCompleter::InlineCompletion);
    searchCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    searchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    searchCompleter->setWrapAround(false);
    searchLineEdit->setCompleter(searchCompleter);

    filterLineEdit->setCompleter(searchCompleter);

    QFontMetrics fm1(searchLineEdit->font());
    searchLineEdit->setMinimumWidth(fm1.averageCharWidth()*32);
    editHighlighter = new EditHighLighter(searchLineEdit->document());
    filterEditHighlighter  = new EditHighLighter(filterLineEdit->document());
    connect(searchLineEdit,SIGNAL(textChanged()),this,SLOT(searchTextChangedSlot()));
    connect(searchLineEdit,SIGNAL(returnPressed()),this,SLOT(searchReturnSlot()));
    searchBox->addWidget(searchL,0);
    searchBox->addWidget(searchLineEdit,1);
    //searchToolBar->addAction(linkSearchA);
    searchToolBar->addWidget(searchLV);
    searchToolBar->addWidget(searchArea);
    searchToolBar->addAction(saveSearchFuncA);
    searchToolBar->addAction(searchBeginA);
    searchToolBar->addAction(searchBackA);
    searchToolBar->addAction(searchNextA);
    searchToolBar->addAction(searchEndA);
    searchToolBar->addAction(searchEditA);
    searchToolBar->addWidget(searchSelectArea);


    whatsThisA = QWhatsThis::createAction(this);
    whatsThisA->setIcon(QIcon(":/images/svg/help-contents.svg"));
    whatsThisA->setToolTip("Provides brief description of  icons and buttons");

    QHBoxLayout *space = new QHBoxLayout();
    space->addStretch(1);
    space->setMargin(0);
    QWidget *spaceW = new QWidget();
    spaceW->setLayout(space);

    searchArea->setLayout(searchBox);
    searchToolBar->addWidget(spaceW);
    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks);
    consoleDock = new QDockWidget(tr("Console"),this);
    consoleDock->setObjectName("ConsoleDock");
    consoleArea = new QTextBrowser(consoleDock);
    addDockWidget(Qt::BottomDockWidgetArea,consoleDock);

    QPalette palette;
    QBrush brush(QColor(255, 170, 0, 255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
    QBrush brush1(QColor(252, 240, 228, 255));
    brush1.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::Text, brush1);
    QBrush brush2(QColor(0, 0, 0, 255));
    brush2.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::Base, brush2);
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
    palette.setBrush(QPalette::Inactive, QPalette::Text, brush1);
    palette.setBrush(QPalette::Inactive, QPalette::Base, brush2);
    QBrush brush3(QColor(126, 125, 124, 255));
    brush3.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush3);
    palette.setBrush(QPalette::Disabled, QPalette::Text, brush3);
    QBrush brush4(QColor(255, 255, 255, 255));
    brush4.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Disabled, QPalette::Base, brush4);
    consoleArea->setPalette(palette);
    consoleArea->setMaximumHeight(340);
    consoleDock->setWidget(consoleArea);
    QFont fnt = consoleDock->font();
    fnt.setBold(true);
    consoleDock->setFont(fnt);
    consoleDock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::TopDockWidgetArea);
    consoleDock->setFloating(false);

    hideColumMenu = new QMenu(this);
    hideColumMenu->setTitle(tr("Columns"));
    hideColumMenu->setTearOffEnabled(true);
    hideColActionGroup = new QActionGroup(this);
    hideColActionGroup->setExclusive(false);
    connect(hideColActionGroup,SIGNAL(triggered (QAction *)),this,SLOT(hideColumnActionSlot(QAction *)));

    configureIconsMenu = new QMenu(tr("Icons"),this);
    iconSizeMenu = new QMenu(tr("Size"),this);
    iconSizeSmallA = new QAction(tr("Small"),this);
    iconSizeRegA    = new QAction(tr("Regular"),this);
    iconSizeLargeA = new QAction(tr("Large"),this);
    iconSizeSmallA ->setCheckable(true);
    iconSizeRegA->setCheckable(true);
    iconSizeLargeA->setCheckable(true);
    iconSizeMenu->addAction(iconSizeSmallA);
    iconSizeMenu->addAction(iconSizeRegA);
    iconSizeMenu->addAction(iconSizeLargeA);
    iconSizeActionGroup = new QActionGroup(this);
    iconSizeActionGroup->setExclusive(true);
    connect(iconSizeActionGroup,SIGNAL(triggered (QAction *)),this,SLOT(iconSizeSlot(QAction *)));
    iconSizeActionGroup->addAction(iconSizeSmallA);
    iconSizeActionGroup->addAction(iconSizeRegA);
    iconSizeActionGroup->addAction(iconSizeLargeA);

    iconStyleMenu = new QMenu(tr("Style"),this);
    iconsOnlyA = new QAction(tr("Icons Only"),this);
    iconsWithTextA = new QAction(tr("Icons+Text"),this);
    iconsTextOnlyA = new QAction (tr("Text Only"),this);
    iconsOnlyA->setCheckable(true);
    iconsWithTextA->setCheckable(true);
    iconsWithTextA->setCheckable(true);
    iconStyleMenu->addAction(iconsOnlyA);
    iconStyleMenu->addAction(iconsWithTextA);
    iconStyleMenu->addAction(iconsTextOnlyA);
    iconsStyleGroup = new QActionGroup(this);
    connect(iconsStyleGroup,SIGNAL(triggered (QAction *)),this,SLOT(iconStyleSlot(QAction *)));
    iconsStyleGroup->setExclusive(true);
    iconsStyleGroup->addAction(iconsOnlyA);
    iconsStyleGroup->addAction(iconsWithTextA);
    iconsStyleGroup->addAction(iconsTextOnlyA);

    hideConsoleA = consoleDock->toggleViewAction();
    configureIconsMenu->addMenu(iconSizeMenu);
    configureIconsMenu->addMenu(iconStyleMenu);
    optionMenu->addAction(hideToolBarA);
    optionMenu->addAction(searchToolBarA);
    optionMenu->addAction(hideConsoleA);
    optionMenu->addAction(editSchemaA);

    optionMenu->addAction(filterToolBarA);

    optionMenu->addAction(filterSenderMenuA);
    optionMenu->addAction(showMessageA);
    optionMenu->setTearOffEnabled(true);
    optionMenu->addMenu(hideColumMenu);
    optionMenu->addMenu(configureIconsMenu);

    windowMenu->addAction(windowNameA);
    windowMenu->addAction(copyTabA);
    windowMenu->addAction(newTabA);
    windowMenu->addAction(copyWindowA);
    windowMenu->addAction(newWindowA);
    windowMenu->addAction(closeA);

    fileMenu->addAction(autoSaveA);
    exportMenu = fileMenu->addMenu("Export Table");
    exportMenu->setToolTip(("Export selected table applying any filters"));
    exportCSVA = exportMenu->addAction("CSV Format");
    exportXLSXA = exportMenu->addAction("XLSX Format");
    exportActionGroup = new QActionGroup(this);
    exportActionGroup->addAction(exportCSVA);
    exportActionGroup->addAction(exportXLSXA);

    connect(exportActionGroup,SIGNAL(triggered(QAction*)),this,SLOT(exportSlot(QAction *)));
    fileMenu->addSeparator();
    fileMenu->addAction(quitA);


    schemaMenu->addAction(editSchemaA);
    mainToolBar->addAction(closeA);
    mainToolBar->addAction(newWindowA);
    mainToolBar->addAction(copyWindowA);
    mainToolBar->addAction(newTabA);
    mainToolBar->addAction(copyTabA);
    mainToolBar->addAction(showMessageA);
    mainToolBar->addAction(autoSaveA);
    mainToolBar->addSeparator();
    mainToolBar->addAction(editSchemaA);
    mainToolBar->addAction(filterSenderMenuA);
    mainToolBar->addAction(filterToolBarA);

    mainToolBar->addAction(searchToolBarA);
    mainToolBar->addSeparator();
    mainToolBar->addAction(fontIncreaseA);
    mainToolBar->addAction(fontDecreaseA);
    mainToolBar->addAction(fontRegularA);
    mainToolBar->addAction(whatsThisA);
    QWidget      *fixVersionArea = new QWidget(mainToolBar);
    QHBoxLayout  *fixVerLayout = new QHBoxLayout(fixVersionArea);
    fixVerLayout->setMargin(0);

    fix8versionL = new QLabel("Schema:",fixVersionArea);
    fix8versionL->setToolTip("What should this tool tip say ?");

    fix8versionV = new QLabel("???",fixVersionArea);
    QPalette fpal = fix8versionV->palette();
    fix8RegColor = fpal.color(QPalette::WindowText); // save for latter
    fix8versionV->setToolTip("And What should this tool tip say ?");
    fnt = fix8versionL->font();
    fnt.setBold(true);
    fix8versionL->setFont(fnt);
    fix8versionV->setFont(fnt);
    fixVerLayout->addStretch(1);
    fixVerLayout->addWidget(fix8versionL,0,Qt::AlignRight);
    fixVerLayout->addWidget(fix8versionV,0);
    mainToolBar->addWidget(fixVersionArea);
    QToolButton *tb = qobject_cast <QToolButton *>(mainToolBar->widgetForAction(filterSenderMenuA));
    if (tb) {
        tb->setPopupMode(QToolButton::InstantPopup);
    }
    // helpMenu
    aboutA = new QAction("About",this);
    aboutQTA = new QAction("About Qt",this);
    connect(aboutQTA,SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    //whatsThis = new QWhatsThis();

    helpMenu->addAction(whatsThisA);
    helpMenu->addAction(aboutA);
    helpMenu->addAction(aboutQTA);

    configPB = new PushButtonModifyKey(this);
    configPB->setToolTip("Change menubar color.\n(Hold Shift Key down to change text color)");
    configPB->setIcon(QIcon(":/images/svg/preferences-color.svg"));
    configPB->setFlat(true);

    menuBar()->setCornerWidget(configPB);
    QWidget *schemaArea = new QWidget(this);
    QHBoxLayout *schemaBox = new QHBoxLayout(schemaArea);
    schemaBox->setMargin(0);
    schemaBox->setSpacing(0);
    schemaArea->setLayout(schemaBox);
    scopeV = new QLabel();
    scopeV->setPixmap(QPixmap(":/images/svg/worldwWthTwoTabs.svg").scaledToHeight(24));
    schemaL = new QLabel("Schema: ",this);
    schemaL->setToolTip("Current Schema");
    schemaV = new QLabel("Default",this);
    schemaV->setToolTip("Schema Scope");
    schemaBox->addWidget(scopeV);
    schemaBox->addSpacing(15);
    schemaBox->addWidget(schemaL);
    schemaBox->addWidget(schemaV);
    schemaV->setToolTip("Current Schema");
    statusBar()->addWidget(schemaArea);
    connect(configPB,SIGNAL(clicked()),this,SLOT(configSlot()));
    connect(configPB,SIGNAL(rightButtonPressed()),this,SLOT(configFGSlot()));
    // restore should be in settings but must come after
    stackW = new QStackedWidget(this);
    setCentralWidget(stackW);
    noDataL = new NoDataLabel("No Data Files\nLoaded");
    connect(noDataL,SIGNAL(modelDropped(FixMimeData*)),this,SLOT(modelDroppedSlot(FixMimeData*)));
    fnt = noDataL->font();
    fnt.setBold(true);
    fnt.setPointSize(fnt.pointSize() + 4);
    noDataL->setFont(fnt);
    noDataL->setAutoFillBackground(true);
    pal = noDataL->palette();
    pal.setColor(QPalette::Window,Qt::black);
    pal.setColor(QPalette::WindowText,Qt::white);
    noDataL->setPalette(pal);
    noDataL->setObjectName("No_Data_Label");
    noDataL->setAlignment(Qt::AlignCenter);
    progressView = new QQuickView(QUrl("qrc:qml/loadProgress.qml"));
    qmlObject = progressView->rootObject();
    if (!qmlObject) {
        qWarning() << "qml root object not found" << __FILE__ << __LINE__ ;
    }
    else
        connect(qmlObject,SIGNAL(cancel()),this,SLOT(cancelSessionRestoreSlot()));
    progressView->setResizeMode(QQuickView::SizeRootObjectToView);
    progressWidget = QWidget::createWindowContainer(progressView,this);

    workAreaSplitter = new QSplitter(Qt::Horizontal,this);
    tabW = new QTabWidget(workAreaSplitter);
    workAreaSplitter->addWidget(tabW);

    connect(tabW,SIGNAL(tabCloseRequested(int)),this,SLOT(tabCloseRequestSlot(int)));
    connect(tabW,SIGNAL(currentChanged(int)),this,SLOT(tabCurentChangedSlot(int)));

    // build tabname edit area
    tabNameEditArea = new QWidget(this);
    QHBoxLayout *tabNameBox = new QHBoxLayout(tabNameEditArea);
    tabNameBox->setMargin(0);
    tabNameBox->setSpacing(2);
    tabNameEditArea->setLayout(tabNameBox);
    fnt = tabNameEditArea->font();
    fnt.setPointSize(fnt.pointSize()-1);
    tabNameEditArea->setFont(fnt);
    cancelEditTabNamePB = new QPushButton(this);
    connect(cancelEditTabNamePB,SIGNAL(clicked()),this,SLOT(cancelTabNameSlot()));
    cancelEditTabNamePB->setIcon(QIcon(":/images/svg/cancel.svg"));
    tabNameLineEdit = new QLineEdit(this);
    tabNameLineEdit->setMaxLength(120);
    QRegExp regExp("^[a-z,A-Z,0-9]+\\s?[a-z,A-Z,0-9]+$");
    QValidator *val = new QRegExpValidator(regExp,this);
    tabNameLineEdit->setValidator(val);
    editTabNamePB = new QPushButton(this);
    connect(editTabNamePB,SIGNAL(clicked(bool)),this,SLOT(editTabNameSlot(bool)));
    editTabNamePB->setCheckable(true);
    editTabNamePB->setFlat(true);
    editTabNamePB->setToolTip(tr("Edit name of current tab"));
    QIcon editTabIcon;
    editTabIcon.addPixmap(QPixmap(":/images/svg/checkmark.svg"),QIcon::Normal,QIcon::On);
    editTabIcon.addPixmap(QPixmap(":/images/svg/edittabname.svg"),QIcon::Normal,QIcon::Off);
    editTabNamePB->setIcon(editTabIcon);
    tabNameBox->addWidget(tabNameLineEdit,1);
    tabNameBox->addWidget(cancelEditTabNamePB,0,Qt::AlignRight);
    tabNameBox->addWidget(editTabNamePB,0);
    cancelEditTabNamePB->hide();

    tabNameLineEdit->hide();
    cancelEditTabNamePB->setToolTip("Cancel edit of current tab name");
    editTabNamePB->setToolTip("Edit current tab name");
    connect(tabNameLineEdit,SIGNAL(textChanged(QString)),this,SLOT(tabNameModifiedSlot(QString)));
    connect(tabNameLineEdit,SIGNAL(returnPressed()),this,SLOT(tabNameReturnKeySlot()));

    tabW->setCornerWidget(tabNameEditArea);
    tabW->setTabPosition(QTabWidget::South);
    tabW->setTabsClosable(true);
    stackW->insertWidget(ShowNoDataLabel,noDataL);
    stackW->insertWidget(ShowTab,workAreaSplitter);
    stackW->insertWidget(ShowProgress,progressWidget);
    connect(newTabA,SIGNAL(triggered()),this,SLOT(createTabSlot()));
    connect(newWindowA,SIGNAL(triggered()),this,SLOT(createWindowSlot()));

    popupMenu = new QMenu(this);
    popupCopyHtmlA = new QAction("Copy HTML",this);
    popupCopyTextA = new QAction("Copy Text",this);
    popupMenu->addAction(popupCopyTextA);
    popupMenu->addAction(popupCopyHtmlA);
    popupActionGroup = new QActionGroup(this);
    popupActionGroup->addAction(popupCopyTextA);
    popupActionGroup->addAction(popupCopyHtmlA);
    connect(popupActionGroup,SIGNAL(triggered(QAction*)),this,SLOT(popupMenuActionSlot(QAction*)));
    buildHideColumnMenu();
}
void MainWindow::buildSchemaMenu()
{
    TableSchema *tableSchema;
    // add default schema
    schemaActionGroup = new QActionGroup(this);
    connect(schemaActionGroup,SIGNAL(triggered(QAction*)),this,SLOT(schemaSelectedSlot(QAction *)));
    schemaActionGroup->setExclusive(true);
    schemaMenu->addSection("Available Schemas");
    /*
    if (sharedLib && sharedLib->defaultTableSchema) {
        QAction *action = new QAction( sharedLib->defaultTableSchema->name,this);
        action->setCheckable(true);
        QVariant var;
        var.setValue((void *)  sharedLib->defaultTableSchema);
        action->setData(var);
        schemaActionMap.insert( sharedLib->defaultTableSchema->id,action);
        schemaMenu->addAction(action);
        schemaActionGroup->addAction(action);
    }
*/
    if (!schemaList) {
        qDebug() << "Schema List is null " << __FILE__ << __LINE__;
        return;
    }

    QListIterator <TableSchema *> iter(*schemaList);

    while(iter.hasNext()) {
        tableSchema = iter.next();
        QAction *action = new QAction(tableSchema->name,this);
        action->setCheckable(true);
        QVariant var;
        var.setValue((void *) tableSchema);
        action->setData(var);
        schemaActionMap.insert(tableSchema->id,action);
        schemaMenu->addAction(action);
        schemaActionGroup->addAction(action);
    }
}
MainWindow::~MainWindow()
{

}
void MainWindow::createWindowSlot()
{
    emit createWindow(this);
}

void MainWindow::showFileDialog()
{
    createTabSlot();
}
void MainWindow::buildHideColumnMenu()
{
    /*
    for (int i=0;i<FixTable::NumColumns;i++) {
        QAction *hideA = new QAction(tr("Hide ") + FixTable::headerLabel[i],this);
        hideA->setCheckable(true);
        hideA->setData(i);
        hideColumMenu->addAction(hideA);
        hideColActionGroup->addAction(hideA);
    }
    */
}
void MainWindow::displayMessageDialog(QString &message)
{
    QString title;
    if (name.length() < 0) {
        title  = qApp->applicationDisplayName();
    }
    QMessageBox::information(this,title,message);
}
QString MainWindow::getName()
{
    return name;
}
const QUuid &MainWindow::getUuid()
{
    return uuid;
}
void MainWindow::closeEvent(QCloseEvent *ce)
{
    ce->ignore();
    closeSlot();
}

void MainWindow::showEvent(QShowEvent *se)
{
    if (!loadingActive)  {
        if (tabW->count() > 0) {
            stackW->setCurrentIndex(ShowTab);
            copyTabA->setEnabled(true);
            showMessageA->setEnabled(true);
        }
        else {
            stackW->setCurrentIndex(ShowNoDataLabel);
            copyTabA->setEnabled(false);
            showMessageA->setEnabled(false);
        }
    }
    validateSearchButtons();
    QMainWindow::showEvent(se);
}
void MainWindow::timerEvent(QTimerEvent *te)
{

}
QSize MainWindow::sizeHint() const
{
    // TODO: figure out default size based on pixel density
    return QSize(800,900);
}
WindowData MainWindow::getWindowData()
{
    WindowData wd;
    wd.menubarStyleSheet = mainMenuBar->styleSheet();
    wd.geometry = this->saveGeometry();
    wd.state    = this->saveState();
    wd.id       = this->windowDataID;
    wd.isVisible = this->isVisible();
    wd.currentTab = tabW->currentIndex();
    wd.tableSchema = tableSchema;
    if (tableSchema)
        wd.tableSchemaID = tableSchema->id;
    else if(defaultTableSchema)
        wd.tableSchemaID = defaultTableSchema->id;
    wd.name     = name;
    wd.searchAll = linkSearchOn;
    wd.searchFunction = searchFunction;
    if (sharedLib)
        wd.fix8sharedlib = sharedLib->fileName;
    QFont fnt = font();
    wd.fontPtSize = fnt.pointSize();

    return wd;
}
void MainWindow::setWindowData(const WindowData wd)
{
    QAction *action;
    QVariant var;
    TableSchema *ts;
    QList <QAction *> al;
    windowDataID = wd.id;
    restoreGeometry(wd.geometry);
    restoreState(wd.state);
    menuBarStyleSheet = wd.menubarStyleSheet;
    if (mainMenuBar) {
        mainMenuBar->setStyleSheet(wd.menubarStyleSheet);
        fileMenu->setStyleSheet(menuStyle);
        optionMenu->setStyleSheet(menuStyle);
        schemaMenu->setStyleSheet(menuStyle);
        helpMenu->setStyleSheet(menuStyle);
    }
    setVisible(wd.isVisible);
    setWindowTitle(wd.name);
    name = wd.name;
    tableSchema  = wd.tableSchema;
    if (!tableSchema) {
        tableSchema = defaultTableSchema;
    }
    if (!tableSchema) {
        qWarning() << "!!! MAIN WINDOW HAS NULL TABLE SCHEMA " << __FILE__ << __LINE__;
        schemaV->setText("???");
        return;
    }
    schemaV->setText(tableSchema->name);
    if (!schemaActionGroup) {
        qWarning() << "Error - No  schemas action items found" << __FILE__ << __LINE__;
    }
    else {
        al = schemaActionGroup->actions();
        if (al.count() > 0) {
            QListIterator <QAction *> iter(al);
            while(iter.hasNext()) {
                action = iter.next();
                var = action->data();
                ts = (TableSchema *) var.value<void *>();
                if (ts == tableSchema) {
                    action->setChecked(true);
                    break;
                }
            }
        }
    }
    setTableSchema(tableSchema);
    linkSearchOn = wd.searchAll;
    //linkSearchA->setChecked(linkSearchOn);
    searchFunction = wd.searchFunction;

    searchLineEdit->setText(searchFunction.function);
    if (wd.fontPtSize > -1) {
        QFont fnt = font();
        fnt.setPointSize(wd.fontPtSize);
        setFont(fnt);
    }

}
QList <WorkSheetData> MainWindow::getWorksheetData(int windowID)
{
    QList <WorkSheetData> wsdList;
    WorkSheetData  wsd;
    if (tabW->count() > 0) {
        for (int i=0; i < tabW->count();i++) {
            WorkSheet *ws =  qobject_cast <WorkSheet *> (tabW->widget(i));
            if (ws) {
                wsd  = ws->getWorksheetData();
                wsd.windowID = windowID;
                wsdList.append(wsd);
            }
        }
    }
    return wsdList;
}
WorkSheetData MainWindow::getWorksheetData(QUuid &workSheetID, bool *ok)
{
    WorkSheetData  wsd;
    *ok = false;
    if (tabW->count() > 0) {
        for (int i=0; i < tabW->count();i++) {
            WorkSheet *ws =  qobject_cast <WorkSheet *> (tabW->widget(i));
            if (ws && (ws->getID() == workSheetID)) {
                wsd  = ws->getWorksheetData();
                *ok = true;
                break;
            }
        }
    }
    return wsd;
}
void MainWindow::addWorkSheet(WorkSheetData &wsd)
{
    //qDebug() << " ???NEW CODE, REMOVE OLD ONE" << __FILE__ << __LINE__;
    int index;
    bool bstatus;
    QString str;
    quint32   returnStatus = 0;
    QList <GUI::ConsoleMessage> messageList;
    QFileInfo fi(wsd.fileName);
    if (!fi.exists()) {
        GUI::ConsoleMessage msg("File named " + wsd.fileName + " was not found",
                                GUI::ConsoleMessage::ErrorMsg);
        displayConsoleMessage(msg);
        return;
    }
    setCursor(Qt::BusyCursor);

    WorkSheet *workSheet = new WorkSheet(this);
    workSheet->setSharedLib(sharedLib);
    workSheet->setTableSchema(tableSchema);
    connect(workSheet,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
            this,SLOT(setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat)));
    connect(workSheet,SIGNAL(modelDropped(FixMimeData *)),
            this,SLOT(modelDroppedSlot(FixMimeData *)));
    connect(workSheet,SIGNAL(sendMessage(GUI::ConsoleMessage)),
            this,SLOT(displayMessageSlot(GUI::ConsoleMessage)));
    connect(workSheet,SIGNAL(terminateCopy(WorkSheet*)),
            this,SLOT(terminatedWorkSheetCopySlot(WorkSheet*)));
    connect(workSheet,SIGNAL(rowSelected(int)),
            this,SLOT(rowSelectedSlot(int)));
    connect(workSheet,SIGNAL(doPopup(const QModelIndex &, const QPoint &)),
            this,SLOT(doPopupSlot(const QModelIndex &, const QPoint &)));
    workSheet->setWindowID(uuid);
    workSheet->setWorkSheetData(wsd);
    str = wsd.fileName;
    if (str.length() > 36) {
        str = "..." + str.right(33);
    }
    index = tabW->addTab(workSheet,str);
    /*
    QPixmap pm(24,24);
    pm.fill(Qt::blue);
    tabW->setTabIcon(index,QIcon(pm));
    */
    tabW->setToolTip(wsd.fileName);
    tabW->setCurrentWidget(workSheet);
    stackW->setCurrentWidget(workAreaSplitter);
    //workSheet->setUpdatesEnabled(false);
    bstatus = workSheet->loadFileName(wsd.fileName,messageList,returnStatus);
    if (!bstatus) {
        if (returnStatus == WorkSheet::TERMINATED) {
            str = "Loading of file: " + wsd.fileName + " was terminated.";
            GUI::ConsoleMessage msg(str);
            statusBar()->showMessage(str,3000);
            messageList.append(str);
            tabW->removeTab(index);
            // don't need to delete it, deletion of tab will delete it
        }
        else if (returnStatus == WorkSheet::CANCEL) {
            str = "Loading of file " + wsd.fileName + " canceled.";
            GUI::ConsoleMessage msg(str);
            statusBar()->showMessage(str,3000);
            messageList.append(msg);
            tabW->removeTab(index);
            workSheet->deleteLater();
        }
        else if (returnStatus == WorkSheet::FILE_NOT_FOUND) {
            tabW->removeTab(index);
            delete workSheet;
            str = "Loading of file " + wsd.fileName + " failed. File not found.";
            GUI::ConsoleMessage msg(str);
            statusBar()->showMessage(str,3000);
            messageList.append(msg);
            delete workSheet;
        }
        else {
            tabW->removeTab(index);
            delete workSheet;
            str = "Loading of file " + wsd.fileName + " failed.";
            GUI::ConsoleMessage msg(str);
            messageList.append(msg);
            statusBar()->showMessage(str,3000);
        }
        //workSheet->setUpdatesEnabled(true);

    }
    else {
        workSheet->setMessageAreaExpansion(MessageArea::HeaderItem,wsd.headerExpanded);
        workSheet->setMessageAreaExpansion(MessageArea::FieldsItem,wsd.fieldsExpanded);
        workSheet->setMessageAreaExpansion(MessageArea::TrailerItem,wsd.trailerExpanded);
       // workSheet->setUpdatesEnabled(true);

        workSheetList.append(workSheet);
        str = "Loading of file " + wsd.fileName + " Completed";
        GUI::ConsoleMessage msg(str,GUI::ConsoleMessage::InfoMsg);
        messageList.append(msg);
        statusBar()->showMessage(str,3000);
    }

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
        tabW->setCurrentIndex(0);
        workSheet = qobject_cast<WorkSheet *> (tabW->widget(0));
        if (workSheet) {
            QMenu *senderMenu = workSheet->getSenderMenu();
            if (filterSenderMenuA && senderMenu) {
                filterSenderMenuA->setMenu(senderMenu);
            }
        }
    }
    else {
        stackW->setCurrentWidget(noDataL);
        copyTabA->setEnabled(false);
        showMessageA->setEnabled(false);
    }
    unsetCursor();
}
void MainWindow::addWorkSheet(WorkSheetModel *model,WorkSheetData &wsd)
{
    int currentIndex = stackW->currentIndex();
    stackW->setCurrentIndex(ShowProgress);
    WorkSheet *newWorkSheet;
    if (!model) {
        qWarning() << "Failed to add work sheet, as model is null" <<  __FILE__ << __LINE__;
        stackW->setCurrentIndex(currentIndex);
        return;
    }
    newWorkSheet = new WorkSheet(model,wsd,this);
    newWorkSheet->setMessageAreaExpansion(MessageArea::HeaderItem,wsd.headerExpanded);
    newWorkSheet->setMessageAreaExpansion(MessageArea::FieldsItem,wsd.fieldsExpanded);
    newWorkSheet->setMessageAreaExpansion(MessageArea::TrailerItem,wsd.trailerExpanded);
    newWorkSheet->setTableSchema(tableSchema);
    workSheetList.append(newWorkSheet);
    newWorkSheet->setWindowID(uuid);
    connect(newWorkSheet,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
            this,SLOT(setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat)));
    connect(newWorkSheet,SIGNAL(modelDropped(FixMimeData *)),
            this,SLOT(modelDroppedSlot(FixMimeData *)));
    connect(newWorkSheet,SIGNAL(sendMessage(GUI::ConsoleMessage)),
            this,SLOT(displayMessageSlot(GUI::ConsoleMessage)));
    connect(newWorkSheet,SIGNAL(terminateCopy(WorkSheet*)),
            this,SLOT(terminatedWorkSheetCopySlot(WorkSheet*)));
    connect(newWorkSheet,SIGNAL(rowSelected(int)),
            this,SLOT(rowSelectedSlot(int)));
    connect(newWorkSheet,SIGNAL(doPopup(const QModelIndex &, const QPoint &)),
            this,SLOT(doPopupSlot(const QModelIndex &, const QPoint &)));
    QString str = wsd.fileName;
    if (wsd.tabAlias.length() > 0)
        str = wsd.tabAlias;
    if (str.length() > 36) {
        str = "..." + str.right(33);
    }
    int index = tabW->addTab(newWorkSheet,str);
    tabW->setToolTip(wsd.fileName);
    if (tabW->count() > 0) {
        copyTabA->setEnabled(true);
        showMessageA->setEnabled(true);
        if (tabW->count() > 1)
            tabW->setCurrentIndex(index);
    }
    stackW->setCurrentIndex(ShowTab);


    if (tabW->count() > 0)
        stackW->setCurrentWidget(workAreaSplitter);
    else
        stackW->setCurrentWidget(noDataL);
}
void MainWindow::setAutoSaveOn(bool on)
{
    autoSaveA->setChecked(on);
}
void MainWindow::setCurrentTabAndSelectedRow(int currentTab, int currentRow)
{
    if ((tabW->count() < 1) || (tabW->count() < currentTab))
        return;
    tabW->setCurrentIndex(currentTab);
}
void MainWindow::popupMenuSlot(const QModelIndex &,const QPoint &)
{

}
void MainWindow::finishDrop(WorkSheetData &wsd, FixMimeData *fmd)
{
    if (!fmd) {
        qWarning() << "Failed to finish drop, fmd = 0" << __FILE__ << __LINE__;
    }
    addWorkSheet(fmd->model,wsd);
}
void MainWindow::setTableSchema(TableSchema *newTableSchema)
{
    WorkSheet *ws;
    tableSchema = newTableSchema;
    QAction *action;
    QVariant var;
    TableSchema *ts;
    QList <QAction *> al;
    if (!tableSchema)
        tableSchema = defaultTableSchema;
    schemaV->setText(tableSchema->name);

    if (!schemaActionGroup) {
        qWarning() << "Error - No  schemas action items found" << __FILE__ << __LINE__;
        return;
    }
    setCursor(Qt::BusyCursor);

    QString colName;
    QStringList colNameList;
    int rowCount = tableSchema->fieldNames.count();
    for(int i=0;i<rowCount;i++) {
        colName  = tableSchema->fieldNames[i];
        colNameList.append(colName);

    }
    colNameList.sort();
    QStringListModel *strModel = 0;
    QStringList fieldNameList;
    if (fieldUsePairList) {
        //qDebug() << "FIELD USE PAIR LIST COUNT = " << fieldUsePairList->count() << __FILE__ << __LINE__;
        QListIterator  <QPair<QString ,FieldUse *>> fieldPairIter(*fieldUsePairList);
        while(fieldPairIter.hasNext()) {
            QPair<QString,FieldUse *>  pair = fieldPairIter.next();
            fieldNameList << pair.first;
        }
        //qDebug() << "FILE NAME LIST COUNT:" << fieldNameList.count();
        strModel = new QStringListModel(fieldNameList,this);
        searchCompleter->setModel(strModel);
    }

    al = schemaActionGroup->actions();
    if (al.count() > 0) {
        QListIterator <QAction *> iter(al);
        while(iter.hasNext()) {
            action = iter.next();
            var = action->data();
            ts = (TableSchema *) var.value<void *>();
            if (ts->id == tableSchema->id) {
                action->setChecked(true);
                break;
            }
        }
    }
    QListIterator <WorkSheet *> iter2(workSheetList);
    while(iter2.hasNext()) {
        ws = iter2.next();
        ws->setTableSchema(tableSchema);
    }
    unsetCursor();
}
void MainWindow::tableSchemaModified(TableSchema *ts)
{
    WorkSheet *ws;
    QAction *action;
    if (!ts) {
        qWarning() << "Error MainWindow::tableSchemaModfied" << __FILE__ << __LINE__;
        return;
    }
    if (!ts->fieldList) {
     qWarning() << "FIELD LIST IS NULL for table modified" << __FILE__ << __LINE__;
     return;
    }
    QMap<int,QAction *>::const_iterator iter  = schemaActionMap.find(ts->id);
    if (iter != schemaActionMap.end()) {
        action = (QAction *) iter.value();
        action->setText(ts->name);
    }

    if (tableSchema->id == ts->id) {
        *tableSchema = *ts;
    }
    QListIterator <WorkSheet *> iter2(workSheetList);
    while(iter2.hasNext()) {
        ws = iter2.next();
        ws->setTableSchema(tableSchema);
    }
}
void MainWindow::addNewSchema(TableSchema *ts)
{
    if (!ts)
        return;
    QAction *action = new QAction(ts->name,this);
    action->setCheckable(true);
    QVariant var;
    var.setValue((void *) ts);
    action->setData(var);
    schemaMenu->addAction(action);
    schemaActionMap.insert(ts->id,action);
    schemaActionGroup->addAction(action);
}
void MainWindow::deletedSchema(int schemaID)
{
    QAction *schemaAction;
    QMap<int, QAction *>::const_iterator i = schemaActionMap.find(schemaID);
    schemaAction = i.value();
    if (schemaAction) {
        schemaMenu->removeAction(schemaAction);
        schemaActionGroup->removeAction(schemaAction);
    }
}
TableSchema * MainWindow::getTableSchema()
{
    return tableSchema;
}
void MainWindow::setFieldUsePair(QList<QPair<QString ,FieldUse *>> *fup)
{
    fieldUsePairList = fup;
    QStringList fieldNames;
    WorkSheet *ws;
    if (!fieldUsePairList) {
        qWarning() << "Warning Field Use Pair is null " << __FILE__ << __LINE__;
        QListIterator <WorkSheet *> iter2(workSheetList);
        while(iter2.hasNext()) {
            ws = iter2.next();
            ws->setFieldUsePair(fup);
        }
        return;
    }
    QListIterator <QPair<QString ,FieldUse *>> iter(*fieldUsePairList);
    while(iter.hasNext()) {
        QPair<QString,FieldUse *> pair = iter.next();
        fieldNames << pair.first;

    }
    editHighlighter->setColumHeaders(fieldNames);
    filterEditHighlighter->setColumHeaders(fieldNames);
}
QList<QPair<QString ,FieldUse *>> * MainWindow::getFieldUsePair()
{
    return fieldUsePairList;
}
void MainWindow::setSharedLibrary(Fix8SharedLib *f8sl)
{
    sharedLib = f8sl;
    QString message;
    QPalette pal = fix8versionV->palette();
    if (!sharedLib) {
        message = "Error = FIX8 schema library is null";
        GUI::ConsoleMessage msg(message,GUI::ConsoleMessage::ErrorMsg);
        displayConsoleMessage(msg);
        pal.setColor(QPalette::WindowText,Qt::red);
        fix8versionV->setText("?");
        fix8versionV->setToolTip("Error no FIX Schema library provided");
    }
    if (!(sharedLib->isOK)) {
        qDebug() << "SOMETHING WRONG WITH SHARED LIB" << __FILE__ << __LINE__;
        qDebug() << "ERROR MESSAGE = " << sharedLib->errorMessage ;
        GUI::ConsoleMessage msg(sharedLib->errorMessage,GUI::ConsoleMessage::ErrorMsg);
        pal.setColor(QPalette::WindowText,Qt::red);
        fix8versionV->setText(sharedLib->name);
        fix8versionV->setToolTip("Error loading FIX schema lib: " + sharedLib->fileName);
        displayConsoleMessage(msg);
        defaultTableSchema = sharedLib->getDefaultTableSchema();
        schemaList = sharedLib->getTableSchemas();

    }
    else {
        message = "FIX8 Schema lib set to " + sharedLib->name;
        GUI::ConsoleMessage msg(sharedLib->errorMessage,GUI::ConsoleMessage::InfoMsg);
        displayConsoleMessage(msg);
        fix8versionV->setText(sharedLib->name);
        fix8versionV->setToolTip(sharedLib->fileName);
        pal.setColor(QPalette::WindowText,fix8RegColor);
    }
    schemaList = sharedLib->tableSchemas;
    buildSchemaMenu();
    fix8versionV->setPalette(pal);
}
Fix8SharedLib * MainWindow::getSharedLibrary()
{
    return sharedLib;
}
