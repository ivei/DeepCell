#include <QApplication>
#include <QDateTime>
#include <QLayout>
#include <QDir>
#include "./frame/cellcounter.h"
#include "./srv/savelog.h"
#include "./srv/dbgutility.h"

#pragma execution_character_set("utf-8")

CellCounter *pCellCounter = nullptr;

CellCounter *getCellCounterWnd()
{
    return pCellCounter;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef QT_NO_DEBUG
    SaveLog::Instance()->start();
#else
    SaveLog::Instance()->stop();
#endif
    TRACE() << "Begin Run at" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    CellCounter w;
    pCellCounter = &w;
    w.show();

//    Toolkit kit;
//    kit.show();

    return a.exec();
}
