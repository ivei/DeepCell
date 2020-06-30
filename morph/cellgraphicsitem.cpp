#include "cellgraphicsitem.h"
#include <QPen>
#include <QMenu>
#include <QFile>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QFont>
#include <QFontMetrics>
#include <QFontMetricsF>
#include "../srv/dbgutility.h"
#include "../srv/deepservice.h"
#include "../srv/deeplabel.h"
#include "../srv/commdefine.h"

#pragma execution_character_set("utf-8")

//! 定义不同类别的颜色. 第一个是线的颜色, 第二个是标题的文字颜色.
static QVector< std::pair<QColor, QColor> > colorDef{
            std::make_pair(Qt::white, Qt::black),   //0
            std::make_pair(Qt::red, Qt::white),  // 1
            std::make_pair(Qt::green,   Qt::black),  //2
            std::make_pair(Qt::blue,    Qt::white),  // 3
            std::make_pair(Qt::darkRed, Qt::white),  // 4
            std::make_pair(Qt::cyan,    Qt::black),  // 5
            std::make_pair(Qt::yellow,  Qt::black),  // 6
            std::make_pair(Qt::magenta, Qt::black),  // 7
            std::make_pair(Qt::darkGreen,Qt::black),  // 8
            //std::make_pair(Qt::darkYellow,Qt::white),  // 9
            std::make_pair(QColor(85,102,0), Qt::white),    // 9
            std::make_pair(Qt::darkBlue,    Qt::white),  // 10
            std::make_pair(Qt::darkGray,    Qt::white),  // 11
            //std::make_pair(Qt::lightGray,   Qt::black),  // 12
            std::make_pair(QColor(160,82,45),   Qt::black),  // 12
            std::make_pair(Qt::darkMagenta, Qt::white),  // 13
            std::make_pair(Qt::darkCyan,    Qt::white),  // 14
            std::make_pair(Qt::gray,        Qt::white),  // 15
            std::make_pair(Qt::black,       Qt::white),  // 16
            std::make_pair(QColor(139,69,0), Qt::white), // 17
            std::make_pair(QColor("#FF9900"), Qt::white)
};


CellGraphicsItem::CellGraphicsItem(const CellItem& item, DeepLabel *handler/*, const QMap<int, CategoryItem> &categorys*/, bool showMark, bool showCounter)
    : QGraphicsPolygonItem(nullptr)
    , QObject()
    //, QObject (nullptr)
    , item(item)
    //, _categorys(categorys)
    , _showMark(showMark)
    , _showCounter(showCounter)
    , _handler(handler)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    initialize();
}

void CellGraphicsItem::initialize()
{
    setPolygon(item.countour());
    setColor(item.type());
    //TRACE() << "rect is: " << this->item.rect();
}

CellItem CellGraphicsItem::getItem() const
{
    return this->item;
}



void CellGraphicsItem::setCategoryId(int categoryId)
{
    item.setCategoryId(categoryId);
    setColor(categoryId);
    update();
}


int CellGraphicsItem::getCategoryId() const
{
    return item.value()._category_id;
}

QRectF CellGraphicsItem::boundingRect() const
{
    //return QGraphicsPolygonItem::boundingRect().adjusted(-5, -40, 5, 5);
    QRect rect = this->item.rect();
    //TRACE() << "rect is: " << rect;
    return rect.adjusted(-5, -40, 5, 5);
}

//! 用于View和Scene的itemAt().
//! 使用细胞的区域(boundingrect)作为shape. 避免笔误画出来的细胞选不中.
QPainterPath CellGraphicsItem::shape() const
{
#if 0
    if( this->polygon().isEmpty())
    {
        QPainterPath path;
        path.addRect(this->item.rect());
        return path;
    }
    else {
        return QGraphicsPolygonItem::shape();
    }
#else
    QPainterPath path;
    path.addRect(this->item.rect());
    return path;
#endif
}




/**
 * @brief 设置Item的画笔的颜色和宽度.
 * @param type
 */
void CellGraphicsItem::setColor(int type)
{
    QPen pen;
    pen.setColor(getPenColor(type));
    pen.setWidth(5);
    setPen(pen);
}

QColor CellGraphicsItem::getPenColor(int type)
{
#if 0
    int id = type % colorDef.size();
    QColor col = colorDef.value(id, std::make_pair(Qt::white, Qt::black)).first;
#else
    int id = type % PALATTE::ColorPalette.size();
    QColor col = QColor(PALATTE::ColorPalette[id].firstColor);
#endif
    col.setAlpha(127);
    TRACE() << "type=" << type << "col=" << col << col.rgb();
    return col;
}

QColor CellGraphicsItem::getFontColor(int type)
{
#if 0
    int id = type % colorDef.size();
    return colorDef.value(id, std::make_pair(Qt::white, Qt::black)).second;
#else
    int id = type % PALATTE::ColorPalette.size();
    QColor color = QColor(PALATTE::ColorPalette[id].secondColor);
    return color;
#endif
}


void CellGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    painter->setPen(pen());

    // 画细胞轮廓
    if( this->_showCounter)
    {
        if( this->item.countour().isEmpty())
        {
            // 尝试读取缓存数据
            //QByteArray data = DeepService::loadContour(this->item.slideNo(), this->item.contourId());
            QByteArray data = DeepService::loadRawData(DeepService::makeContourCacheName(this->item.slideNo(), this->item.contourId()));
            if( data.isEmpty())
            {
                auto reply = this->_handler->fetchCellContours(this->item.contourId());
                connect(reply, &DeepLabelReply::finished, this, &CellGraphicsItem::onDownloadFinished);
            }
            else
            {
                this->item.loadContours(data);
                this->setPolygon(this->item.countour());
            }
        }

        if( !this->item.countour().isEmpty())
        {
            if (option->state & QStyle::State_Selected){
                painter->setBrush(QBrush(QColor(255, 255, 0, 64)));
            }
            else{
                painter->setBrush(brush());
            }
            painter->drawPolygon(polygon(), fillRule());
        }

    }


    // 画框标记并写类别
    if( this->_showMark )
    {
        QRectF rect = boundingRect();
        painter->drawRect(rect);

        QRectF titleRect = QRectF(rect.topLeft(), QSizeF(rect.width(), 35));
        painter->drawRect(titleRect);
        QColor fillColor = pen().color();
        //fillColor.setAlpha(200);
        painter->fillRect(titleRect, QBrush(fillColor));

        // 写字.

        QString title = this->titleText();
        //TRACE() << "title is: " << title;
        QFont font("Arial", 20);
        painter->setFont(font);
        painter->setPen(getFontColor(this->item.type()));

        painter->drawText(titleRect, Qt::AlignCenter, title);
    }
}

//! 设置是否绘制细胞的边框, 如果标志改变了, 就要重画.
void CellGraphicsItem::setShowMark(bool isShow)
{
    if( isShow == this->_showMark){
        return;
    }
    else {
        this->_showMark = isShow;
        //  仅强制重画可见的.  不可见的Item的重画在paint事件中处理.
        if( this->isVisible())
            this->update();
    }
}

void CellGraphicsItem::setShowCounter(bool isShow)
{
    if( isShow != this->_showCounter)
    {
        this->_showCounter = isShow;
        if( this->isVisible())
            this->update();
    }
}

void CellGraphicsItem::onDownloadFinished()
{
    TRACE();
    DeepLabelReply* reply = qobject_cast<DeepLabelReply *>(sender());
    if( reply->error() != DeepLabelReply::NoError)
    {
        ERROR() << "Failed to download contours for object " << this->item.objectId();
        return;
    }
    QByteArray data = reply->rawData();
    this->item.loadContours(data);
    this->setPolygon(this->item.countour());
    this->update();
    // cache数据.
    //DeepService::cacheContour(this->item.slideNo(), this->item.contourId(), data);
    DeepService::cacheRawData(DeepService::makeContourCacheName(this->item.slideNo(), this->item.contourId()), data);

    reply->deleteLater();
}

