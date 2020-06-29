#ifndef TILITEM_H
#define TILITEM_H
#include <QGraphicsItem>
#include <QObject>

class TileGraphicItem: public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    enum { Type = UserType + 3 };
    int type() const override    {
         // Enable the use of qgraphicsitem_cast with this item.
         return Type;
    }
    enum ImageType{ TypeTile=1, TypeNormal};
    //! 创建瓦片模式的Item
    TileGraphicItem(const QString& base_path, const QString& suffix, int x, int y, int levels, QObject *parent=nullptr);
    //! 创建普通图的图项
    TileGraphicItem(const QString& path, const QSize& size, QObject* parent=nullptr);
    ~TileGraphicItem() override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void render(const QString& base_path, int level, int x, int y, const QString& suffix);

public slots:
    void updatePixmap(const QPixmap& pixmap, int level);

private:
    ImageType   imageType;
    QString     path;
    QString     suffix;
    QSize       imageSize;
    QPoint      offset;
    int         levels;
    QPixmap     pixmap;
    int         currentLevel;
    QRectF      roi;
};

#endif // TILITEM_H
