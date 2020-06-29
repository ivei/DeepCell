#include "downloadmanager.h"
#include "deeplabel.h"
#include <QQueue>
#include <QTimer>

#pragma execution_character_set("utf-8")

class DownloadManagerPrivate
{
public:
    DeepLabel manager;
    QQueue<QString> downloadQueue;
    int current;
    int total;
    QList<QByteArray> results;
};

DownloadManager::DownloadManager(QObject *parent):QObject(parent), data(new DownloadManagerPrivate)
{
    data->manager.setHost("192.168.4.249");
    data->manager.setCredential("sanner", "666666", "user");
}

DownloadManager::~DownloadManager()
{
    delete data;
}

void DownloadManager::append(const QString& urlString)
{
    if (data->downloadQueue.empty())
    {
        QTimer::singleShot(0, this, &DownloadManager::downloadNext);
    }

    data->downloadQueue.append(urlString);
    data->total++;
}

void DownloadManager::append(const QStringList& urlList)
{
    for(auto url : urlList)
    {
        append(url);
    }
}

void DownloadManager::downloadNext()
{
    if (data->downloadQueue.isEmpty()) {
        emit finished();
        return;
    }

    auto url = data->downloadQueue.dequeue();
    auto reply = data->manager.download(url);

    connect(reply, &DeepLabelReply::finished, this, &DownloadManager::downloadFinished);
    data->current++;
}

void DownloadManager::downloadFinished()
{
    auto reply = qobject_cast<DeepLabelReply*>(sender());
    if (reply->error() != DeepLabelReply::NoError)
    {
        return;
    }

    data->results.append(reply->rawData());
    reply->deleteLater();
    downloadNext();
}

void DownloadManager::downloadReadyRead()
{

}

QList<QByteArray> DownloadManager::getResults() const
{
    return data->results;
}
