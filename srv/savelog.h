#ifndef SAVELOG_H
#define SAVELOG_H

#include <QObject>
#include <QDate>
class QFile;

class SaveLog : public QObject
{
    Q_OBJECT
public:
    //! 声明为Singleton
    static SaveLog *Instance();
    virtual ~SaveLog();
    explicit SaveLog(QObject *parent = nullptr);

signals:

public slots:
    void start();   //! 启动日志保存重定向
    void stop();    //! 关闭日志重定向
    //! 保存日志内容
    void save(const QString& content);
    //! 设置日志保存位置
    //! 设置日志文件模式下保存的路径
    //! 设置保存日志文件时的日志文件名称
private:
    static QScopedPointer<SaveLog> self;

    QFile *file;
    QString path;   //! 日志路径
    QString name;   //! 日志名称
    QDate   date;
    QString fileName;   //! 日志文件全路径名
};

#endif // SAVELOG_H
