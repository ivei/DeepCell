#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QObject>
#include <QMap>
#include <functional>

class DeepLabel;
class ImageProvider: public QObject
{
    Q_OBJECT
public:
    ImageProvider(DeepLabel *handler);

signals:
    void ready(const QString& path, const QPixmap& pixmap);
public slots:
    void request(const QString& path);
    void onReplyFinised(const QString& path);
private:
    DeepLabel *handler;
    QStringList pending;
    QMap<QString, QPixmap> cached;
};

#endif // IMAGEPROVIDER_H
