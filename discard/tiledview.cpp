#include <QGraphicsScene>
#include "ui_tiledview.h"

#include "cellgraphicsitem.h"
#include "cellitem.h"
#include "dbgutility.h"
#include "deeplabel.h"
#include "maptileitem.h"
#include "maptileprovider.h"
#include "taskresult.h"
#include "tiledview.h"
#include "categoryitem.h"

#pragma execution_character_set("utf-8")

TiledView::TiledView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TiledView)
    , _cellMarkShow(true)
    , _cellCounterShow(true)
{
    ui->setupUi(this);
    ui->widget->setVisible(false);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

}

TiledView::~TiledView()
{
    delete ui;
}

void TiledView::initialize(const TaskResult &result, DeepLabel* handler)
{
    this->categorys = result.catalogs();    // 保存翻译字典

    auto sc = new QGraphicsScene(this);

    if (result.imageList().empty())
    {
        return;
    }
    // 对骨髓来说, 就只有一个视野. imageList就只有一个元素, 就是first.
    image = result.imageList().first();
    auto provider = new MapTileProvider(handler, image.path());
    auto size = image.size();
    //每个瓦片的尺寸固定为 = 256 * 256

    // 下载缩略图, 并在toolkit上面显示.
    TRACE() << "download thumbnail: " << image.thumbnail();
    auto reply = handler->download(image.thumbnail());
    reply->waitForFinished();
    QPixmap pixmap;
    pixmap.loadFromData(reply->rawData());
    qreal ratio = qMax(qreal(pixmap.width()) / size.width(), qreal(pixmap.height()) / size.height());
    ui->graphicsView->setThumbnail(pixmap, ratio);

    // 下载和显示瓦片图像. 每个瓦片构造为一个BufferedPixmapItem对象.
    auto levels = int(std::ceil(std::log(qMax(size.width(), size.height())) / std::log(2))) + 1;
    sc->setSceneRect(- size.width() / 2, - size.height() / 2, size.width() * 2, size.height() * 2);
    for (int x = 0; x < size.width(); x+=256)
    {
        for (int y = 0; y < size.height(); y+=256)
        {
            auto item = new BufferedPixmapItem(x/256, y/256, levels, sc);
            connect(item, &BufferedPixmapItem::render, provider, &MapTileProvider::request);
            item->setPos(x, y);
            sc->addItem(item);
        }
    }
    // 每个细胞是一个CellGraphicsItem对象实例. 添加到Scene中, 同时还保存在字典items里面. items是以细胞id为key的Map.
    // 使用items似乎是为了便于在TiledView中管理, 不然就要通过graphicsView访问到scene, 再在scene中和瓦片的item一起
    // 查找, 要复杂得多.
    for (const CellItem& item: result.cellList())
    {
        auto i = new CellGraphicsItem(item, this->cellMarkShow(), this->cellCounterShow());
        sc->addItem(i);
        items.insert(item.id(), i);
    }
    ui->graphicsView->setScene(sc);


}

QImage TiledView::capture(const QRect& rect)
{
    QImage image(rect.size(), QImage::Format_RGB888);
    image.fill(Qt::black);
    QPainter painter(&image);

    auto sc = ui->graphicsView->scene();
    auto items = sc->items(rect);
    for(auto iter = items.begin(); iter != items.end();)
    {
        if (dynamic_cast<CellGraphicsItem*>(*iter) && (*iter)->isVisible())
        {
            (*iter++)->hide();
        }
        else
        {
            iter = items.erase(iter);
        }
    }

    sc->render(&painter, image.rect(), rect);
    std::for_each(items.begin(), items.end(), [](QGraphicsItem* item){
       item->show();
    });

    return image;
}

ImageView *TiledView::view()
{
    return ui->graphicsView;
}

bool TiledView::cellMarkShow() const
{
    return this->_cellMarkShow;
}

bool TiledView::cellCounterShow() const
{
    return this->_cellCounterShow;
}

/**
 * @brief 增加细胞的处理.
 * @param item
 */
void TiledView::onCellAdded(const CellItem& item)
{
    if (items.contains(item.id()))
    {
        return;
    }
    auto i = new CellGraphicsItem(item, this->cellMarkShow(), this->cellCounterShow());
    ui->graphicsView->scene()->addItem(i);
    items.insert(item.id(), i);
}

// 处理细胞被选中的处理： 将细胞挪到视图的正中， 并同步更新toolkit的状态。
void TiledView::onCellSelected(const CellItem& item)
{
    TRACE();
    if (!items.contains(item.id()))
    {
        return;
    }
    ui->graphicsView->centerOn(items.value(item.id()));
    ui->graphicsView->updateToolKitRegion();
}

void TiledView::onCellDeleted(const CellItem& item)
{
    qDebug() << "to be deleted" << item.id();
    if (!items.contains(item.id()))
    {
        return;
    }
    qDebug() << "delete" << item.id();

    auto i = items[item.id()];
    items.remove(item.id());
    ui->graphicsView->scene()->removeItem(i);
    delete i;
}

/**
 * @brief 处理细胞类型发生改变的处理
 * @param item  细胞
 * @param type  新的类别
 */
void TiledView::onCellTypeChanged(const CellItem& item, int type)
{
    if (!items.contains(item.id()))
    {
        return;
    }
    auto i = items[item.id()];
    auto typeItem = this->categorys.value(type);

    TRACE() << static_cast<CellGraphicsItem*>(i)->id() << type;
    //static_cast<CellGraphicsItem*>(i)->setType(type);
    CellGraphicsItem *pItem = qgraphicsitem_cast<CellGraphicsItem *>(i);
    pItem->setCategory(typeItem);
    pItem->update();

}

void TiledView::onCellShowMarkChanged(bool isShow)
{
    TRACE();
    for( QGraphicsItem* item: ui->graphicsView->scene()->items())
    {
        CellGraphicsItem* it = qgraphicsitem_cast<CellGraphicsItem *>(item);
        if( it != nullptr){
            it->setShowMark(isShow);
        }
    }

}

void TiledView::onCellShowCounterChanged(bool isShow)
{
    TRACE();
    for( QGraphicsItem* item: ui->graphicsView->scene()->items())
    {
        CellGraphicsItem* it = qgraphicsitem_cast<CellGraphicsItem *>(item);
        if( it != nullptr){
            it->setShowCounter(isShow);
        }
    }
}
