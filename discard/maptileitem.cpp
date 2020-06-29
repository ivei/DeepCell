#include "maptileitem.h"
#include "maptileprovider.h"
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QtMath>
#include <QDebug>

#pragma execution_character_set("utf-8")

BufferedPixmapItem::BufferedPixmapItem(int x, int y, int levels, QObject* parent): QObject(parent), offset(x, y), levels(levels), currentLevel(-1)
{
    setFlag(ItemIsSelectable, false);
}

BufferedPixmapItem::~BufferedPixmapItem()
{

}

QRectF BufferedPixmapItem::boundingRect() const
{
    return QRectF(0, 0, 256, 256);
}

void BufferedPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
    int level = qBound(0, int(qLn(lod)/ qLn(0.5)), levels - 1);
    double ratio = pow(0.5, level);
    int x = qFloor(offset.x() * ratio);
    int y = qFloor(offset.y() * ratio);
    qreal dx = (offset.x() * ratio - x) * 256;
    qreal dy = (offset.y() * ratio - y) * 256;

    if (!pixmap.isNull() && level == currentLevel)
    {
        roi = QRectF(dx, dy, 256 * ratio, 256 * ratio);
        painter->drawPixmap(option->rect, pixmap,  roi);
    }
    else
    {
        emit render(levels - level - 1, x, y);
        painter->drawPixmap(option->rect, pixmap.copy(roi.toRect()));
    }
}

void BufferedPixmapItem::updatePixmap(const QPixmap& pixmap, int level)
{
    currentLevel = levels - level -1;
    this->pixmap = pixmap;
    update();
}

