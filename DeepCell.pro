#-------------------------------------------------
#
# Project created by QtCreator 2019-12-11T09:03:05
#
#-------------------------------------------------

QT       += core gui network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DeepCell
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    frame/batchmodel.cpp \
    frame/cellcounter.cpp \
    frame/cellitemdeletate.cpp \
    frame/connectiondialog.cpp \
    frame/connectionitem.cpp \
    frame/connectionitemdelegate.cpp \
    frame/connetionmodel.cpp \
    frame/simpleprogressdlg.cpp \
    frame/tasklistmodel.cpp \
    frame/tasklistview.cpp \
    frame/taskproxymodel.cpp \
    frame/unassignedtasklistmodel.cpp \
    morph/addcommand.cpp \
    morph/analyzerwindow.cpp \
    morph/catalogview.cpp \
    morph/cellgraphicsitem.cpp \
    morph/cellgroupbox.cpp \
    morph/celllistmodel.cpp \
    morph/celllisttypefiltermodel.cpp \
    morph/cellshowpage.cpp \
    morph/changetypecommand.cpp \
    morph/deletecommand.cpp \
    morph/imagedialog.cpp \
    morph/imageview.cpp \
    morph/reportdialog.cpp \
    morph/reportdlg.cpp \
    morph/reporttemplatemodel.cpp \
    morph/tilegraphicitem.cpp \
    morph/tileprovider.cpp \
    morph/toolkit.cpp \
    morph/celllistview.cpp \
    srv/afxapp.cpp \
    srv/cellitem.cpp \
    srv/datalist.cpp \
    srv/deepengine.cpp \
    srv/deeplabel.cpp \
    srv/deeplabelreply.cpp \
    srv/deepservice.cpp \
    srv/imageitem.cpp \
    srv/slideitem.cpp \
    srv/savelog.cpp \
    srv/taskresult.cpp \
    srv/utilities.cpp \
    main.cpp \
    frame/splashwndext.cpp \
    frame/aboutdialog.cpp \
    srv/category.cpp


HEADERS += \
    frame/batchmodel.h \
    frame/cellcounter.h \
    frame/cellitemdeletate.h \
    frame/connectiondialog.h \
    frame/connectionitem.h \
    frame/connectionitemdelegate.h \
    frame/connetionmodel.h \
    frame/simpleprogressdlg.h \
    frame/tasklistmodel.h \
    frame/tasklistview.h \
    frame/taskproxymodel.h \
    frame/unassignedtasklistmodel.h \
    morph/addcommand.h \
    morph/analyzerwindow.h \
    morph/catalogview.h \
    morph/cellgraphicsitem.h \
    morph/cellgroupbox.h \
    morph/celllistmodel.h \
    morph/celllisttypefiltermodel.h \
    morph/reportdlg.h \
    morph/changetypecommand.h \
    morph/cellshowpage.h \
    morph/deletecommand.h \
    morph/imagedialog.h \
    morph/imageview.h \
    morph/reportdialog.h \
    morph/reporttemplatemodel.h \
    morph/tilegraphicitem.h \
    morph/tileprovider.h \
    morph/toolkit.h \
    morph/celllistview.h \
    srv/afxapp.h \
    srv/cellitem.h \
    srv/commdefine.h \
    srv/datalist.h \
    srv/dbgutility.h \
    srv/deepengine.h \
    srv/deeplabel.h \
    srv/deeplabelreply.h \
    srv/deepservice.h \
    srv/imageitem.h \
    srv/savelog.h \
    srv/slideitem.h \
    srv/taskresult.h \
    srv/utilities.h \
    frame/splashwndext.h \
    frame/aboutdialog.h \
    version.h \
    srv/category.h


FORMS += \
    morph/analyzerwindow.ui \
    morph/catalogview.ui \
    morph/cellgroupbox.ui \
    morph/imagedialog.ui \
    morph/reportdialog.ui \
    morph/reportdlg.ui \
    morph/toolkit.ui \
    frame/cellcounter.ui \
    frame/connectiondialog.ui \
    frame/connectionitem.ui \
    frame/tasklistview.ui \
    frame/aboutdialog.ui

RC_FILE += app.rc
# Default rules for deployment.
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

DISTFILES += \
    app.rc

win32:CONFIG(release, debug|release): LIBS += -L$$(OPENCV)/x64/vc15/lib/ -lopencv_world344
else:win32:CONFIG(debug, debug|release): LIBS += -L$$(OPENCV)/x64/vc15/lib/ -lopencv_world344d

INCLUDEPATH += $$(OPENCV)/include
DEPENDPATH += $$(OPENCV)/include
