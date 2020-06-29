#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QDebug>
#include "../srv/dbgutility.h"
#include "../srv/deeplabel.h"
#include "../srv/deepservice.h"
#include "../srv/imageitem.h"
#include "tileprovider.h"
#include "tilegraphicitem.h"
#pragma execution_character_set("utf-8")

#if 0
TileProvider::TileProvider(DeepLabel* handler, const QString&path)
    : handler(handler)
    , path(path)
{

}
#endif

TileProvider::TileProvider(DeepLabel *handler)
    : handler(handler)
{

}

TileProvider::~TileProvider()
{
    TRACE();
    INFO() << "cache size: " << this->cached.size();
    this->cached.clear();
    INFO() << "cache size: " << this->cached.size();
}
#if 0
void TileProvider::request(int level, int x, int y)
{
    TileGraphicItem* item = qobject_cast<TileGraphicItem *>(sender());
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
    connect(reply, &DeepLabelReply::finished, this, std::bind(&TileProvider::onReplyFinised, this, id, level));
}
#endif

/**
 * @brief 下载请求
 * @param base_path: 瓦片情况下的基础路径.
 * @param x : 瓦片的x方向序号
 * @param y : 瓦片的y方向序号
 * @param level
 * @note
 *    - 我们的实现中, 在imageView中, 不管Image如何缩放, 其中的图项个数是不变的,
 *      这样, 一个瓦片实际上会对应一个或几个图项. 因此, 一次下载图像可以提供给几个
 *      图项使用.
 *    - 这里的x和y并不是图项的索引, 而是瓦片的索引, 是在图项的paint()函数中计算出来
 *      的. 这一点需要注意.
 *
 *    - 对于非瓦片的情况, x,y,level都是-1, base_path就是最终要下载的文件全路径, 不需要
 *      再构造了.
 */
void TileProvider::request(const QString &base_path, int level, int x, int y, const QString &suffix)
{
    //TRACE() << "base_path=" << base_path << ", level=" << level << "x=" << x << ", y=" << y;
    TileGraphicItem *item = qobject_cast<TileGraphicItem *>(sender());
    QString server_path;
    // 构造逻辑.根据x,y,level是否等于-1来判断是否是一个外周血. 这个以后再优化.
    if( x==-1 && y==-1 && level==-1){
        server_path = base_path;
    }
    else{
        // 把路径转换为tile路径.
        server_path = ImageItem::makeTileFilePath(base_path, level, x, y, suffix);
        // 老的方案
        //file_path = QString("%1/dzi/%2/%3_%4.jpg").arg(base_path).arg(level).arg(x).arg(y);
    }
    // 缓存是以文件路径来标识的.
    if( cached.contains(server_path))
    {
        item->updatePixmap(cached.value(server_path), level);
        return;
    }

    // 准备读取server_path所指向的资源. 如果资源在本地存在, 则读取文件;
    // 如果不存在, 就读取网络.
    // 读取文件就同步完成, 读取网络就异步执行.
#if 1
    QString file_path = DeepService::makeResourceCacheName(server_path);
    QByteArray result = DeepService::loadRawData(file_path);
#else
    QByteArray result = DeepService::readCache(server_path, DeepService::PathType::ResourceFile);
#endif
    if( !result.isEmpty())
    {
        //INFO() << "use cache for image/tile: " << server_path;
        QPixmap pixmap;
        pixmap.loadFromData(result);
        cached.insert(server_path, pixmap);
        item->updatePixmap(pixmap, level);
        return;
    }
    else
    {
        pending[server_path].append(item);
        if( pending[server_path].count() > 1){
            return;
        }

        //INFO() << "download " << server_path;
        auto reply = handler->download(server_path);
        connect(reply, &DeepLabelReply::finished, this, std::bind(&TileProvider::onReplyFinised, this, server_path, level));
    }
}

/**
 * @brief 处理下载完成后的处理.
 * @param path  文件路径
 * @param level
 */
void TileProvider::onReplyFinised(const QString& path, int level)
{
    //TRACE() << "finished download tile image at level= " << level << "path=" << path;
    DeepLabelReply* reply = qobject_cast<DeepLabelReply *>(sender());
    // 任务列表中找不到的时候
    auto it = pending.find(path);
    if (it == pending.end())
    {
        reply->deleteLater();
        return;
    }

    if (reply->error() != DeepLabelReply::NoError)
    {
        ERROR() << QString("Failed to download from %1: %2").arg(path).arg(reply->errorString());
        pending.erase(it);
        reply->deleteLater();
        return;
    }

    QPixmap pixmap;
    QByteArray data = reply->rawData();
    pixmap.loadFromData(data);
    cached.insert(path, pixmap);

    for (auto req : it.value())
    {
        req->updatePixmap(pixmap, level);
    }
    pending.erase(it);
#if 0
    DeepService::writeCache(path, data, DeepService::PathType::ResourceFile);
#else
    DeepService::cacheRawData(DeepService::makeResourceCacheName(path), data);
#endif
    reply->deleteLater();
}
