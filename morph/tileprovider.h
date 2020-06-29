#ifndef TILEPROVIDER_H
#define TILEPROVIDER_H

#include <QObject>
#include <QPixmap>
#include <QMap>

class DeepLabel;
class TileGraphicItem;

class TileProvider: public QObject
{
    Q_OBJECT
public:
    //TileProvider(DeepLabel *handler, const QString& path);
    TileProvider(DeepLabel* handler);
    ~TileProvider();
public slots:
    //void request(int x, int y, int level);
    void request(const QString& base_path, int level, int x, int y, const QString& suffix);
    void onReplyFinised(const QString& path, int level);
signals:

private:
    DeepLabel *handler;
    //QString path;
    // 任务队列. key是下载路径. 值是使用这个路径的ITEM的列表.
    QMap<QString, QList<TileGraphicItem*>> pending;
    // 已经下载的图片的列表. key是图片的下载路径.
    QMap<QString, QPixmap> cached;
};

#endif // TILEPROVIDER_H
