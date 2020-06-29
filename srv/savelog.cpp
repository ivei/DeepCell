#include <QApplication>
#include <QDate>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QDir>
#include "savelog.h"
#include "dbgutility.h"

#pragma execution_character_set("utf-8")

//! 定义全局的日志记录钩子函数
//! void Log
void logHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context)
    QMutex mutex;
    QMutexLocker locker(&mutex);
    QString content;

    switch (type) {
    case QtDebugMsg:
        content = QString("%1").arg(msg);
        break;
    case QtWarningMsg:
        content = QString("%1").arg(msg);
        break;
    case QtCriticalMsg:
        content = QString("%1").arg(msg);
        break;
    case QtFatalMsg:
        content = QString("%1").arg(msg);
        break;
    }
    SaveLog::Instance()->save(content);
}

QScopedPointer<SaveLog> SaveLog::self;
SaveLog* SaveLog::Instance()
{
    // 双检查枷锁模式，凑合着用了。
    if( self.isNull()){
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if( self.isNull()){
            self.reset(new SaveLog);
        }
    }
    return self.data();
}

SaveLog::~SaveLog()
{
    TRACE();
    this->file->close();
}

void SaveLog::start()
{
    qInstallMessageHandler(logHandler);
}

void SaveLog::stop()
{
    qInstallMessageHandler(nullptr);
}

void SaveLog::save(const QString &content)
{
    // 当前日期. 一天记录一个日志文件, 如果不同了, 就更换.
    QDate date(QDate::currentDate());
    if( date != this->date)
    {
        this->date = date;
        QString fileName = QString("%1/log/%2_log_%3.log")
                .arg(this->path)
                .arg(this->name)
                .arg(qUtf8Printable(date.toString("yyyy-MM-dd")));
        this->fileName = fileName;
        if( this->file->isOpen()){
            this->file->close();
        }
        this->file->setFileName(fileName);
        this->file->open(QIODevice::WriteOnly|QIODevice::Append|QFile::Text);
    }

    QTextStream logStream(file);
    logStream << QString(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
              << content << "\n";

}

SaveLog::SaveLog(QObject *parent) : QObject(parent)
{
    this->file = new QFile(this);
    this->path = qApp->applicationDirPath();

    // 日志文件名称的主体部分是应用的名字
    this->name = qApp->applicationName().split("/").last().split(".").at(0);
    QDir baseDir(this->path);
    baseDir.mkdir("log");

}
