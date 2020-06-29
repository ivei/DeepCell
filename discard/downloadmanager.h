#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QFutureWatcher>
#include <QObject>

class DownloadManagerPrivate;

class DownloadManager: public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject* parent=nullptr);
    ~DownloadManager() override;
    void append(const QString&);
    void append(const QStringList&);

    QList<QByteArray> getResults() const;

signals:
    void finished();
    void progress(const QString&);
    void error(const QString&);

protected slots:
    void downloadNext();
    void downloadFinished();
    void downloadReadyRead();
private:
    DownloadManagerPrivate *data;
};

#endif // DOWNLOADMANAGER_H
