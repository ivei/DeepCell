#include "imageprovider.h"
#include <deeplabel.h>
#include <QPixmap>
#include "dbgutility.h"
#pragma execution_character_set("utf-8")

ImageProvider::ImageProvider(DeepLabel *handler): handler(handler)
{

}

void ImageProvider::request(const QString& path)
{
    if (cached.contains(path))
    {
        emit ready(path, cached.value(path));
        return;
    }

    if (pending.contains(path))
    {
        return;
    }
    pending << path;

    auto reply = handler->download(path);
    connect(reply, &DeepLabelReply::finished, this, std::bind(&ImageProvider::onReplyFinised, this, path));
}

void ImageProvider::onReplyFinised(const QString& path)
{
    DeepLabelReply* reply = qobject_cast<DeepLabelReply *>(sender());

    if (!pending.contains(path))
    {
        reply->deleteLater();
        return;
    }

    if (reply->error() == DeepLabelReply::NoError)
    {
        QPixmap pixmap;
        pixmap.loadFromData(reply->rawData());
        cached.insert(path, pixmap);
        emit ready(path, pixmap);
    }
    else {
        TRACE() << "download failed for" << path <<": "<< reply->errorString();
    }
    pending.removeAll(path);
    reply->deleteLater();
}

