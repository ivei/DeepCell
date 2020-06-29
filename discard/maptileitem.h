#ifndef MAPTILITEM_H
#define MAPTILITEM_H
#include <QGraphicsItem>
#include <QObject>

class BufferedPixmapItem: public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    enum { Type = UserType + 3 };

    int type() const override
    {
         // Enable the use of qgraphicsitem_cast with this item.
         return Type;
    }
    BufferedPixmapItem(int x, int y, int levels, QObject *parent=nullptr);
    ~BufferedPixmapItem() override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void render(int level, int x, int y);

public slots:
    void updatePixmap(const QPixmap& pixmap, int level);

private:
    QPoint offset;
    int levels;
    QPixmap pixmap;
    int currentLevel;
    QRectF roi;
};

#endif // TILEDPIXMAPITEM_H
