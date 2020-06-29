#include "graphicsimageitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "dbgutility.h"

#pragma execution_character_set("utf-8")

GraphicsImageItem::GraphicsImageItem(const QString& url, const QSize& size, QObject *parent)
    :QObject(parent), path(url), imageSize(size)
{
    setFlag(ItemIsSelectable, false);
}

GraphicsImageItem::~GraphicsImageItem()
{

}

QRectF GraphicsImageItem::boundingRect() const
{
    return QRect(QPoint(), imageSize);
}

void GraphicsImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    if (pixmap.isNull())
    {
        TRACE() << "need download" << path;
        emit render(path);
    }
    else
    {
        painter->drawPixmap(option->rect, pixmap);
    }
}

void GraphicsImageItem::setPixmap(const QPixmap& pixmap)
{
    this->pixmap = pixmap;
    update();
}

