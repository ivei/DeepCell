#include <QHBoxLayout>
#include <QVBoxLayout>

#include "morph/cellgraphicsitem.h"
#include "morph/imageview.h"
#include "morph/tileprovider.h"
#include "morph/tilegraphicitem.h"

#include "../srv/afxapp.h"
#include "../srv/category.h"
#include "../srv/commdefine.h"
#include "../srv/dbgutility.h"
#include "../srv/deeplabel.h"
#include "../srv/taskresult.h"

#include "cellshowpage.h"
#pragma execution_character_set("utf-8")




CellShowPage::CellShowPage(CellShowPage::WorkMode workmode, QWidget *parent)
    : QWidget(parent)
    , _workMode(workmode)
    , handler(nullptr)
    , provider(nullptr)
    , _cellMarkShow(true)
    , _cellContourShow(false)
    , listWidget(nullptr)
{
    TRACE();
    // 创建图像界面元素
    // 注意, 因为不同工作模式下显示的界面元素不同, 在这里无法确定. 因此,
    // 在setWorkMode()里面会隐藏或显示特有的控件.
    setupUI();

    // 设置图像显示视图的属性. 放在这里是为了强调
    this->graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    this->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    //! listWidget用于指定要显示哪个视野. 每个视野对应一个Scene, 根据视野的路径搜索scene并进行切换.
    //! (老实现机制完全不同, 已经废除)
    connect(this->listWidget, &QListWidget::currentItemChanged, this, &CellShowPage::onCurrentImageChanged);

}

CellShowPage::~CellShowPage()
{
    TRACE();
    for(auto& item: this->imageSet)
    {
        item.scene->clear();
        TRACE() << item.scene->items();
        delete item.scene;
        item.showItemList.clear();
    }
    if( provider)
        delete provider;

}

void CellShowPage::setWorkMode(CellShowPage::WorkMode workmode)
{
    TRACE() << "workMode=" << workmode;
    this->_workMode = workmode;
}

CellShowPage::WorkMode CellShowPage::workMode() const
{
    return this->_workMode;
}

/**
 * @brief 此函数被调用, 以完成初始化工作. 派生类必须先调用它.
 * @param result
 * @param handler
 */
void CellShowPage::initialize(const TaskResult &result, DeepLabel *handler)
{

    TRACE() << QString("Begin initialize CellShowPage. image count=%1, cell count=%2")
               .arg(result.imageList().size()).arg(result.cellList().size());
    this->handler = handler;
    // 保存分类字典
    this->categorys = result.catalogs();
    // 将分类字典添加到ImageView中, 同时构造下拉菜单列表.
    this->view()->setCategoryMap(result.catalogs());


    this->initializeNew(result, handler);

}

QImage CellShowPage::capture(const QRect &rect)
{
    TRACE() << "capture rect: " << rect;
    QImage image(rect.size(), QImage::Format_RGB888);
    image.fill(Qt::black);
    QPainter painter(&image);

    //auto items = this->graphicsScene->items(rect);
    auto items = this->view()->scene()->items(rect);
    for(auto iter=items.begin(); iter!=items.end(); )
    {
        if( qgraphicsitem_cast<CellGraphicsItem*>(*iter)!=nullptr && (*iter)->isVisible() )
        {
            (*iter++)->hide();
        }
        else {
            iter = items.erase(iter );
        }
    }
    //this->graphicsScene->render(&painter, image.rect(), rect);
    this->view()->scene()->render(&painter, image.rect(), rect);
    // 恢复绘制
    std::for_each( items.begin(), items.end(), [](QGraphicsItem* item){
        item->show();
    } );

    return image;
}


const ImageItem &CellShowPage::currentImageData() const
{
    return this->image;
}

bool CellShowPage::cellMarkShow() const
{
    return this->_cellMarkShow;
}

void CellShowPage::setCellMarkShow(bool show)
{
    this->_cellMarkShow = show;
    this->view()->update();
}

bool CellShowPage::cellContourShow() const
{
    return this->_cellContourShow;
}

void CellShowPage::setCellContourShow(bool show)
{
    this->_cellContourShow = show;
    this->view()->update();
}

void CellShowPage::initializeNew(const TaskResult &result, DeepLabel *handler)
{
    TRACE();
    IF_RETURN(result.imageList().isEmpty());

    PRINT_CLOCK(QString("begin initializeNew..."));

    this->provider = new TileProvider(handler);

    for(const auto& image: result.imageList())
    {
        auto size = image.imageSize();
        ImageSetItem imageInfo;
        imageInfo.key   = image.imagePath();
        imageInfo.data  = image;

        // 创建Scene
        QGraphicsScene *cur_scene = new QGraphicsScene(this);
        cur_scene->setSceneRect(0,0, size.width(), size.height());
        imageInfo.scene = cur_scene;

        // 获取并保存缩略图
        auto thumb_item = new QListWidgetItem(QIcon(image.prevPixmap()), QString(), this->listWidget);

        // 把缩略图加到缩略图列表中
        thumb_item->setData(Qt::UserRole, image.imagePath());
        // 保存缩略图到字典中. 数据冗余. 以后考虑优化删掉.
        imageInfo.thumbPixmap = image.prevPixmap();
        // 瓦片和普通图片要分别处理.
        if( image.isTile())
        {
            // 处理瓦片
            auto levels = int( std::ceil( std::log(qMax(size.width(), size.height()))/std::log(2))) + 1;
            for(int x=0; x<size.width(); x+=TILE_SIZE)
            {
                for(int y=0; y<size.height(); y+=TILE_SIZE)
                {
                    auto tile_item = new TileGraphicItem(/*image.imagePath()*/image.tileBaseDir(), image.suffix(), x/TILE_SIZE, y/TILE_SIZE, levels, cur_scene);
                    connect(tile_item, &TileGraphicItem::render, provider,&TileProvider::request);
                    tile_item->setPos(x,y);
                    cur_scene->addItem(tile_item);
                    imageInfo.showItemList.append(tile_item);
                }
            }
        }
        else
        {
            auto image_item = new TileGraphicItem(image.imagePath(), size, this);
            connect(image_item, &TileGraphicItem::render, provider, &TileProvider::request);
            cur_scene->addItem(image_item);
            imageInfo.showItemList.append(image_item);
        }

        for(const CellItem& cell: result.cellList())
        {
            if( cell.imageId() != image.id()){
                continue;
            }

            auto cell_item = new CellGraphicsItem(cell, handler, /*this->categorys, */this->cellMarkShow(),this->cellContourShow());
            cur_scene->addItem(cell_item);

            CellSetItem cellInfo;
            cellInfo.cellId = cell.uid();
            cellInfo.imageId= image.id();
            cellInfo.imagePath = image.imagePath();
            cellInfo.showItem = cell_item;
            cellInfo.scene      = cur_scene;
            cellInfo.data       = cell;
            this->cellSet.insert(cell.uid(), cellInfo);
        }
        this->imageSet.insert(image.imagePath(), imageInfo);
    }  // end of for(...imageList)

    if( this->listWidget->count()>0){
        // 当只有一张视野的时候, 不显示视野列表.
        if( this->listWidget->count()==1){
            this->listWidget->hide();
        }
        this->listWidget->setCurrentRow(0);
    }

    PRINT_CLOCK(QString("finished initializeNew"));
    emit sigNotifyMsg(CellShowPage::NotifyNormal, QString("视野个数%1, 细胞个数%2").arg(this->imageSet.size()).arg(this->cellSet.size()));
}


/**
 * @brief 向视图中增加一个细胞. 当在CatalogView中新增一个细胞时处理.
 * @param item
 */
void CellShowPage::onCellAdded(const CellItem &item)
{
    TRACE() << QString("add new Cell: id=%1, type=%2").arg(item.uid()).arg(item.typeName()) ;
    // 处理重复增加
    IF_RETURN_2(this->cellSet.contains(item.uid()), QString("Error. the cell to add (id=^1) has exist in cell set").arg(item.uid()));


    auto newCell = new CellGraphicsItem(item, this->handler, /*this->categorys,*/ this->cellMarkShow(), this->cellContourShow());
    this->scene()->addItem(newCell);


    CellSetItem cellInfo;
    cellInfo.cellId     = item.uid();
    cellInfo.data       = item;
    cellInfo.scene      = this->view()->scene();
    cellInfo.imageId    = item.imageId();
    cellInfo.imagePath  = this->image.imagePath();
    cellInfo.showItem   = newCell;

    this->cellSet.insert(item.uid(), cellInfo);

    this->imageSet[this->image.imagePath()].showItemList.append(newCell);

    emit sigNotifyMsg(CellShowPage::NotifyNormal, tr("新增细胞..."));
    emit sigNotifyMsg(CellShowPage::NotifyCellNumber, this->cellSet.values().size());


}

/**
 * @brief 处理CatalogView中的细胞被选中的处理.
 * @note
 *      不同的视图中的处理模式是不同的:
 *      - 如果是外周血, GalleryView中, 需要将该细胞对应的视图加载过来.
 *      - 如果是骨髓血, 则只需要将其设置为中心即可.
 * @note
 *      注意, 这个函数也会触发selectionChanged. 因此, selectionChanged事件不能发给CatalogView. 不然
 *      会导致循环.
 *
 */
void CellShowPage::onCellSelected(const CellItem &item)
{
    TRACE() << "Cell id=" << item.uid() << "will select syn.";

    // 根据细胞ID在细胞数据集中查找
    int cellId = item.uid();
    IF_RETURN_2(!this->cellSet.contains(cellId),QString("Cell id=%1 does not in ImageView").arg(item.uid()));
    const auto& cellShowItem = this->cellSet.value(cellId);
    // 从细胞数据集中获取视野的path, 并进而找到视野的数据
    const auto& path = cellShowItem.imagePath;
    // 根据path找到视野数据
    IF_RETURN_2(!this->imageSet.contains(path), QString("Can not find image by path %1").arg(path));
    //const auto& imageShowItem = this->imageSet.value(path);
    // 检查是否需要切换scene.
    if( path != this->image.imagePath())
    {
        this->switchImage(path);
    }

    // 居中显示细胞
    this->centerCell(cellShowItem.showItem);

    sigNotifyMsg(CellShowPage::NotifyNormal, item.showInfo());
    //sigNotifyMsg(CellShowPage::NotifyCellSelect, item.toString());
}

void CellShowPage::deleteCell(int cellId)
{
    TRACE() << QString("delete Cell Id=%1").arg(cellId);
    IF_RETURN_2(!this->cellSet.contains(cellId), QString("can not find cell id=%1!").arg(cellId));

    // 根据cellId查找
    IF_RETURN_2(!this->cellSet.contains(cellId), QString("the cell to delete (cellId=%1) does not in imageview").arg(cellId));
    auto& cellInfo = this->cellSet.value(cellId);
    auto path = cellInfo.imagePath;
    IF_RETURN_2(!this->imageSet.contains(path),
                QString("the cell to delete(cellId=%1, path=%2) does not in image set").arg(cellId).arg(path));

    // 先清理Image中的信息.
    auto& imageInfo = this->imageSet[path];
    imageInfo.showItemList.removeAll(cellInfo.showItem);
    // 再清理Scene:
    auto scene = cellInfo.scene;
    scene->removeItem(cellInfo.showItem);
    delete cellInfo.showItem;
    this->cellSet.remove(cellId);


    emit sigNotifyMsg(CellShowPage::NotifyNormal, tr("删除了细胞"));
    emit sigNotifyMsg(CellShowPage::NotifyCellNumber, this->cellSet.values().size());


}

void CellShowPage::onCellDeleted(const CellItem &item)
{
    TRACE() << item.toString();
    deleteCell(item.uid());

}


void CellShowPage::onCellTypeChanged(const CellItem &item, int type)
{
    TRACE() << QString("change cell id=%1, oldtype=%2(%3) to %4")
               .arg(item.uid()).arg(item.type())
               .arg(item.typeName())
               .arg(type);

    //  直接切换到要修改的细胞所在的视图上进行修改.
    this->onCellSelected(item);
    IF_RETURN_2(!this->cellSet.contains(item.uid()), QString("cell (id=%1) does not exist in image view").arg(item.uid()));
    auto& cellInfo = this->cellSet[item.uid()];
    // 新类型的信息

    auto showItem = qgraphicsitem_cast<CellGraphicsItem *>(cellInfo.showItem);
    IF_RETURN_2(showItem==nullptr, QString("Cell's graphics item is nullptr"));

    showItem->setCategoryId(type);
    showItem->update();

    emit sigNotifyMsg(CellShowPage::NotifyNormal, tr("修改了细胞的类别..."));

}

void CellShowPage::onCellShowMarkChanged(bool isShow)
{
    for(auto& cell: this->cellSet)
    {
        auto item = qgraphicsitem_cast<CellGraphicsItem *>(cell.showItem);
        if(item){
            item->setShowMark(isShow);
            //item->update();
        }
    }

}

void CellShowPage::onCellShowCounterChanged(bool isShow)
{
    for(auto& cell: this->cellSet)
    {
        auto item = qgraphicsitem_cast<CellGraphicsItem *>(cell.showItem);
        if(item){
            item->setShowCounter(isShow);
            //item->update();
        }
    }
}

/**
 * @brief 处理listWidget中的选择的视图发生改变后的处理, 要更新imageView中的显示. 合并瓦片和普通图片的处理.
 * @param current
 * @param previous
 */
void CellShowPage::onCurrentImageChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    if( current)
    {
        TRACE();
        auto path = current->data(Qt::UserRole).toString();

        switchImage(path);
        #if 0
        auto& imageInfo = this->imageSet.value(path);
        if( imageInfo.showItemList.size() > 0){
            centerCell(imageInfo.showItemList.first());
        }
        #endif
    }

}





void CellShowPage::setupUI()
{
    this->resize(1080, 741);
    this->mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0,0,0,0);

    // 创建图像视图
    this->graphicsView = new ImageView(this);
    //this->graphicsView->setContextMenuPolicy(Qt::DefaultContextMenu);
    mainLayout->addWidget(this->graphicsView);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    // 外周血界面上需要显示图像列表.
    // 创建GalleryView使用的列表, 用来显示每个图片的缩略图
    this->listWidget = new QListWidget(this);
    this->listWidget->setSizePolicy(sizePolicy);
    listWidget->setFixedWidth(250);
    listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
    listWidget->setDefaultDropAction(Qt::IgnoreAction);
    listWidget->setMovement(QListView::Static);
    listWidget->setViewMode(QListView::IconMode);
    listWidget->setIconSize(QSize(200,200));

    mainLayout->addWidget(this->listWidget);

}

/**
 * @brief 切换Image.
 * @param key: Image的路径, 被用来唯一标识一个Image.
 */
void CellShowPage::switchImage(const QString &new_path)
{
    TRACE();
    // key是新的Image的路径. 获取Image对应的视图信息结构.
    IF_RETURN_2(!this->imageSet.contains(new_path), QString("Error, Can not find Image for path: %1").arg(new_path));

    const auto& imageShowItem = this->imageSet.value(new_path);
    this->image = imageShowItem.data;
    QSize size = this->image.imageSize();
    this->graphicsScene = imageShowItem.scene;
    const QPixmap& pixmap = imageShowItem.thumbPixmap;
    qreal ration = qMax( qreal(pixmap.width())/size.width(), qreal(pixmap.height())/size.height());
    this->view()->setThumbnail(pixmap, ration);
    this->view()->setScene(this->graphicsScene);
    this->view()->setCellFamily(image.imageType()==ImageItem::TypeMega ? TCellFamily::FamilyMega : TCellFamily::FamilyHema);

    if( imageShowItem.data.imageType()==ImageItem::TypeNormal)
    {
        TRACE() << "FitInView" << this->view()->rect() << "scene: " << this->scene()->sceneRect() << "item: " << imageShowItem.showItemList.first()->boundingRect();
//        if( !first){
//            first = true;
//        }
//        else {
            //this->view()->fitInView(imageShowItem.showItemList.first(), Qt::KeepAspectRatio);
            this->view()->fitInView(this->view()->sceneRect(), Qt::KeepAspectRatio);
//        }
    }
    else if( imageShowItem.data.imageType()==ImageItem::TypeMega){
        //this->view()->fitInView(QRectF(QPoint(), this->image.imageSize()), Qt::KeepAspectRatio);
        this->view()->fitInView(this->view()->sceneRect(), Qt::KeepAspectRatio);
    }


    QString t = this->image.imageTypeText();
    TRACE() << t;
    emit sigNotifyMsg(CellShowPage::NotifyImageType, this->image.imageTypeText());
    emit sigNotifyMsg(CellShowPage::NotifyImageName, this->image.imageName());

}

void CellShowPage::centerCell(QGraphicsItem *cell)
{
    emit sigNotifyMsg(CellShowPage::NotifyNormal, tr("居中显示细胞..."));
    TRACE();
    IF_RETURN_2(cell==nullptr, QString("cell is nullptr"));
    this->view()->centerOn(cell);
    this->view()->updateToolKitRegion();
    this->view()->scene()->clearSelection();
    cell->setSelected(true);
}


