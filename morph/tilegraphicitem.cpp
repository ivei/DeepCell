#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QtMath>
#include <QDebug>

#include "../srv/dbgutility.h"

#include "tilegraphicitem.h"
#include "tileprovider.h"

#pragma execution_character_set("utf-8")

TileGraphicItem::TileGraphicItem(const QString &base_path, const QString& suffix, int x, int y, int levels, QObject* parent)
    : QObject(parent)
    , imageType(TypeTile)
    , path(base_path)
    , suffix(suffix)
    , imageSize(256,256)
    , offset(x, y)
    , levels(levels)
    , currentLevel(-1)
{
    //TRACE() << "path=" << path;
    setFlag(ItemIsSelectable, false);
}

TileGraphicItem::TileGraphicItem(const QString &path, const QSize &size, QObject *parent)
    : QObject(parent)
    , imageType(TypeNormal)
    , path(path)
    , suffix("jpeg")
    , imageSize(size)
    , offset(0,0)
    , levels(-1)
    , currentLevel(-1)
{
    //TRACE() << "path=" << path;
    setFlag(ItemIsSelectable, false);
}

TileGraphicItem::~TileGraphicItem()
{
    //TRACE();
}

QRectF TileGraphicItem::boundingRect() const
{
    return QRectF(QPoint(), imageSize);
}

void TileGraphicItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if( this->imageType==TypeTile)
    {
        //TRACE();
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
            emit render(path, levels - level - 1, x, y, this->suffix);
            painter->drawPixmap(option->rect, pixmap.copy(roi.toRect()));
        }
    }
    else
    {
        if( !pixmap.isNull())
        {
            painter->drawPixmap(option->rect, pixmap);
        }
        else {
            emit render(path, -1, -1, -1, this->suffix);
        }
    }
}

void TileGraphicItem::updatePixmap(const QPixmap& pixmap, int level)
{
    if( this->imageType==TypeTile){
        currentLevel = levels - level -1;
        this->pixmap = pixmap;
    }
    else {
        this->pixmap = pixmap;
    }
    update();
}

