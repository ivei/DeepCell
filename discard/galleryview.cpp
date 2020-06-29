#include "galleryview.h"
#include "ui_galleryview.h"
#include "taskresult.h"
#include "deeplabel.h"
#include "cellgraphicsitem.h"
#include "imageprovider.h"
#include "graphicsimageitem.h"
#include <QListWidgetItem>

#pragma execution_character_set("utf-8")

GalleryView::GalleryView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GalleryView)
    , _cellMarkShow(true)
    , _cellCounterShow(true)
{
    ui->setupUi(this);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    // 下面是和TiledView不同的
    ui->listWidget->setIconSize(QSize(200, 200));
    connect(ui->listWidget, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current, QListWidgetItem *previous){
        if (current && previous)
        {
            auto item = images.value(previous->data(Qt::UserRole).toString());
            item->setVisible(false);
        }
        if (current)
        {
            auto item = images.value(current->data(Qt::UserRole).toString());
            this->image = imageItems.value(current->data(Qt::UserRole).toString());
            item->setVisible(true);
            ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
        }
    });
}

GalleryView::~GalleryView()
{
    delete ui;
}

void GalleryView::initialize(const TaskResult &result, DeepLabel *handler)
{
    this->categorys = result.catalogs();    // 保存翻译字典
    auto sc = new QGraphicsScene(this);

    auto provider = new ImageProvider(handler);

    connect(provider, &ImageProvider::ready, [this](const QString& path, const QPixmap& pixmap){
        static_cast<GraphicsImageItem*>(images.value(path))->setPixmap(pixmap);
    });

    for (const auto& image: result.imageList())
    {
        auto reply = handler->download(image.thumbnail());
        reply->waitForFinished();
        if (reply->error() == reply->NoError)
        {
            QPixmap pixmap;
            pixmap.loadFromData(reply->rawData());
            auto item = new QListWidgetItem(QIcon(pixmap), QString(), ui->listWidget);
            item->setData(Qt::UserRole, image.url());
        }

        auto item = new GraphicsImageItem(image.url(), image.size(), this);
        item->setVisible(false);
        connect(item, &GraphicsImageItem::render, provider, &ImageProvider::request);
        sc->addItem(item);
        images.insert(image.url(), item);
        imageItems.insert(image.url(), image);
        for (const CellItem& cell: result.cellList())
        {
            if (cell.imageId() != image.id())
            {
                continue;
            }
            auto cellItem = new CellGraphicsItem(cell, this->cellMarkShow(), this->cellCounterShow());
            cellItem->setParentItem(item );
            sc->addItem(cellItem);
            items.insert(cell.id(), cellItem);
        }
    }

    if (ui->listWidget->count() > 0)
    {
        ui->listWidget->setCurrentRow(0);
    }

    ui->graphicsView->setScene(sc);
}

QImage GalleryView::capture(const QRect& rect)
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


ImageView *GalleryView::view()
{
    return ui->graphicsView;
}

bool GalleryView::cellMarkShow() const
{
    return this->_cellMarkShow;
}

bool GalleryView::cellCounterShow() const
{
    return this->_cellCounterShow;
}

void GalleryView::onCellAdded(const CellItem& item)
{
    if (items.contains(item.id()))
    {
        return;
    }
    auto i = new CellGraphicsItem(item, this->cellMarkShow(), this->cellCounterShow());
    ui->graphicsView->scene()->addItem(i);
    items.insert(item.id(), i);
}

void GalleryView::onCellSelected(const CellItem&item)
{
    if (!items.contains(item.id()))
    {
        return;
    }
    auto i = items.value(item.id());
    for (auto item: images.values())
    {
        item->setVisible(false);
    }
    i->parentItem()->setVisible(true);
    ui->graphicsView->centerOn(i);
}



void GalleryView::onCellDeleted(const CellItem& item)
{
    if (!items.contains(item.id()))
    {
        return;
    }
    auto i = items[item.id()];
    items.remove(item.id());
    ui->graphicsView->scene()->removeItem(i);
    delete i;
}

void GalleryView::onCellTypeChanged(const CellItem& item, int type)
{
    if (!items.contains(item.id()))
    {
        return;
    }
    auto i = items[item.id()];
    auto typeItem = this->categorys.value(type);

    //static_cast<CellGraphicsItem*>(i)->setCategory(type);
    CellGraphicsItem *pItem = qgraphicsitem_cast<CellGraphicsItem *>(i);
    if( pItem != nullptr){
        pItem->setCategory(typeItem);
        pItem->update();
    }
}

void GalleryView::onCellShowMarkChanged(bool isShow)
{
    for( QGraphicsItem* item: ui->graphicsView->scene()->items())
    {
        CellGraphicsItem* it = qgraphicsitem_cast<CellGraphicsItem *>(item);
        if( it != nullptr){
            it->setShowMark(isShow);
        }
    }
}

void GalleryView::onCellShowCounterChanged(bool isShow)
{
    for( QGraphicsItem* item: ui->graphicsView->scene()->items())
    {
        CellGraphicsItem* it = qgraphicsitem_cast<CellGraphicsItem *>(item);
        if( it != nullptr){
            it->setShowCounter(isShow);
        }
    }
}

