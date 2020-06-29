#include "maptileprovider.h"
#include "maptileitem.h"
#include "deeplabel.h"
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QDebug>
#include "dbgutility.h"

#pragma execution_character_set("utf-8")

MapTileProvider::MapTileProvider(DeepLabel* handler, const QString&path): handler(handler), path(path)
{

}

void MapTileProvider::request(int level, int x, int y)
{
    BufferedPixmapItem* item = qobject_cast<BufferedPixmapItem *>(sender());
    QString id = QString("%1/dzi/%2/%3_%4.jpg").arg(path).arg(level).arg(x).arg(y);
    if (cached.contains(id))
    {
        item->updatePixmap(cached.value(id), level);
        return;
    }

    pending[id].append(item);
    if (pending[id].count() > 1)
    {
        return;
    }
    //TRACE() << "Request download tile image at level=:" << level << "path= " << id;
    auto reply = handler->download(id);
    connect(reply, &DeepLabelReply::finished, this, std::bind(&MapTileProvider::onReplyFinised, this, id, level));
}

void MapTileProvider::onReplyFinised(const QString& path, int level)
{
    //TRACE() << "finished download tile image at level= " << level << "path=" << path;
    DeepLabelReply* reply = qobject_cast<DeepLabelReply *>(sender());

    auto it = pending.find(path);
    if (it == pending.end())
    {
        reply->deleteLater();
        return;
    }

    if (reply->error() != DeepLabelReply::NoError)
    {
        pending.erase(it);
        reply->deleteLater();
        return;
    }

    QPixmap pixmap;
    pixmap.loadFromData(reply->rawData());
    cached.insert(path, pixmap);

    for (auto req : it.value())
    {
        req->updatePixmap(pixmap, level);
    }
    pending.erase(it);
    reply->deleteLater();
}
