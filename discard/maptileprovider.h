#ifndef MAPTILEPROVIDER_H
#define MAPTILEPROVIDER_H

#include <QObject>
#include <QPixmap>
#include <QMap>

class DeepLabel;
class BufferedPixmapItem;

class MapTileProvider: public QObject
{
    Q_OBJECT
public:
    MapTileProvider(DeepLabel *handler, const QString& path);

public slots:
    void request(int x, int y, int level);
    void onReplyFinised(const QString& path, int level);
private:
    DeepLabel *handler;
    QString path;
    QMap<QString, QList<BufferedPixmapItem*>> pending;
    QMap<QString, QPixmap> cached;
};

#endif // TILEDIMAGEPROVIDER_H
