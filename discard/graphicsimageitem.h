#ifndef GRAPHICSIMAGEITEM_H
#define GRAPHICSIMAGEITEM_H
#include <QObject>
#include <QGraphicsItem>

class GraphicsImageItem: public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    enum { Type = UserType + 2 };

    int type() const override
    {
         // Enable the use of qgraphicsitem_cast with this item.
         return Type;
    }
    GraphicsImageItem(const QString& url, const QSize& size, QObject *parent=nullptr);
    ~GraphicsImageItem() override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    bool isReady() const { return !this->pixmap.isNull();}
signals:
    void render(const QString&);

public slots:
    void setPixmap(const QPixmap& pixmap);

private:
    QString path;
    QSize imageSize;
    QPixmap pixmap;
};

#endif // GRAPHICSIMAGEITEM_H
