#-------------------------------------------------
#
# Project created by QtCreator 2014-03-15T21:24:01
#
#-------------------------------------------------

CONFIG += debug_and_release
QT       += xlsx core gui sql qml quick widgets quickwidgets script concurrent
TARGET = fix8logviewer
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    settings.cpp \
    globals.cpp \
    fix8log.cpp \
    fixtable.cpp \
    mainFileSlots.cpp \
    mainWindowSlots.cpp \
    intItem.cpp \
    worksheet.cpp \
    messagefield.cpp \
    messagearea.cpp \
    database.cpp \
    windowdata.cpp \
    databaseWindows.cpp \
    worksheetdata.cpp \
    databaseWorksheets.cpp \
    fix8logDataFile.cpp \
    messageitem.cpp \
    fixHeaderView.cpp \
    dateTimeDelegate.cpp \
    futurereaddata.cpp \
    searchlineedit.cpp \
    fixtoolbar.cpp \
    fixmimedata.cpp \
    nodatalabel.cpp \
    schemaeditordialog.cpp \
    schemaitem.cpp \
    databaseTableSchemas.cpp \
    tableschema.cpp \
    schemaEditorDialogSlots.cpp \
    databaseSchemaFields.cpp \
    fix8logSlots.cpp \
    schemadelegate.cpp \
    selectedfieldstreeview.cpp \
    worksheetmodel.cpp \
    messageitemdelegate.cpp \
    proxyFilter.cpp \
    mainWindowSearch.cpp \
    lineedit.cpp \
    editHighLighter.cpp \
    searchDelegate.cpp \
    fixtableverticaheaderview.cpp \
    worksheetSearches.cpp \
    searchDialog.cpp \
    searchfunction.cpp \
    databaseSearchFunctions.cpp \
    comboboxlineedit.cpp \
    qtlocalpeer.cpp \
    qtlockedfile.cpp \
    qtsingleapplication.cpp \
    qtsinglecoreapplication.cpp \
    pushbuttonmodifykey.cpp \
    schemaeditordialogMessageView.cpp \
    fieldsview.cpp \
    fix8logInit.cpp \
    fix8WindowSlots.cpp \
    newwindowwizard.cpp \
    embeddedfileselector.cpp \
    fix8Database.cpp \
    fix8sharedlib.cpp \
    newwindowschemapage.cpp \
    newwindowfilepage.cpp \
    welcomepage.cpp \
    mainWindowFilter.cpp \
    databaseFilterFunctions.cpp \
    workSheetFilters.cpp \
    logicFilter.cpp \
    mainWindowExport.cpp \
    threadloader.cpp \
    workSheetLoadFileConcurrently.cpp \
    loaddatathread.cpp


HEADERS  += mainwindow.h \
    globals.h \
    fix8log.h \
    fixtable.h \
    intItem.h \
    worksheet.h \
    messagefield.h \
    messagearea.h \
    database.h \
    windowdata.h \
    worksheetdata.h \
    messageitem.h \
    fixHeaderView.h \
    dateTimeDelegate.h \
    futurereaddata.h \
    searchlineedit.h \
    fixtoolbar.h \
    fixmimedata.h \
    nodatalabel.h \
    schemaeditordialog.h \
    schemaitem.h \
    tableschema.h \
    schemadelegate.h \
    selectedfieldstreeview.h \
    worksheetmodel.h \
    messageitemdelegate.h \
    proxyFilter.h \
    lineedit.h \
    editHighLighter.h \
    searchDelegate.h \
    fixtableverticaheaderview.h \
    searchDialog.h \
    searchfunction.h \
    comboboxlineedit.h \
    qtlocalpeer.h \
    QtLockedFile \
    qtlockedfile.h \
    qtsingleapplication.h \
    qtsinglecoreapplication.h \
    pushbuttonmodifykey.h \
    fieldsview.h \
    newwindowwizard.h \
    embeddedfileselector.h \
    fix8sharedlib.h \
    newwindowschemapage.h \
    newwindowfilepage.h \
    welcomepage.h \
    logicFilter.h \
    threadloader.h \
    loaddatathread.h

RESOURCES += \
    resources.qrc
    UI_DIR = ui
    OBJECTS_DIR = obj
unix {
message("Unix Compile.")
QMAKE_CXXFLAGS += -Wno-missing-field-initializers -Wno-ignored-qualifiers -Wno-missing-field-initializers -Wno-uninitialized -Wno-unused-variable -Wno-unused-parameter -std=c++11
QMAKE_CXXFLAGS_RELEASE += -O3
LIBS += -lz  -L/usr/local/lib  -lrt -lfix8 -ltbb  -lPocoFoundation -lPocoNet -lPocoUtil
INCLUDEPATH += /usr/local/include /usr/local/include/fix8
DEPENDPATH += /usr/local/include
SOURCES += qtlockedfile_unix.cpp
OJBS=.obj
MOCS-.moc
}
win32 {
     message("Windows Compile")
    SOURCES += qtlockedfile_win.cpp
    QMAKE_CXXFLAGS += /bigobj -D WIN32_LEAN_AND_MEAN

    INCLUDEPATH += . \
                ../fix8/msvc/packages/fix8.dev.1.3.20140922.1/build/native/include \
                ../fix8/msvc/packages/fix8.dependencies.getopt.1.0.20140509.1/build/native/include \
                ../fix8/msvc/packages/fix8.dependencies.openssl.1.0.20140509.1/build/native/include/x64/v120/Release/Desktop \
                ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/include \
                ../fix8/msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/include

release {
   LIBS +=  rpcrt4.lib \
        ../fix8/msvc/packages/fix8.dev.1.3.20140922.1/build/native/lib/x64/v120/Release/Desktop/fix8.lib \
        ../fix8/msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Release/Desktop/tbb.lib \
        ../fix8/msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Release/Desktop/tbbmalloc_proxy.lib \
        ../fix8/msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Release/Desktop/tbbmalloc.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoFoundation.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoNet.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoNetSSL.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoCrypto.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoUtil.lib
}
debug {
LIBS +=  rpcrt4.lib \
        ../fix8/msvc/packages/fix8.dev.1.3.20140922.1/build/native/lib/x64/v120/Debug/Desktop/fix8d.lib \
        ../fix8/msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Debug/Desktop/tbb_debug.lib \
        ../fix8/msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Debug/Desktop/tbbmalloc_proxy_debug.lib \
        ../fix8/msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Debug/Desktop/tbbmalloc_debug.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoFoundationd.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoNetd.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoNetSSLd.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoCryptod.lib \
        ../fix8/msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoUtild.lib
}
}
OTHER_FILES += \
    qml/loadProgress.qml \
    images/svg/messageView.svg \
    qml/LetterArea.qml \
    qml/helpAbout.qml \
    qml/newMainWindow.qml \
    images/Broken_Link-128.png \
    images/svg/listView.svg \
    qml/welcome.qml \
    qml/schemaLocation.qml

