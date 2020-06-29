#include <QtMath>
#include <QContextMenuEvent>
#include <QGraphicsItem>
#include <QMenu>
#include <QRegion>
#include <QWheelEvent>

#include "cellgraphicsitem.h"
#include "imageview.h"
#include "toolkit.h"
#include "../srv/categoryitem.h"
#include "../srv/dbgutility.h"

#pragma execution_character_set("utf-8")

ImageView::ImageView(QWidget* parent)
    : QGraphicsView(parent)
    , tookit(new Toolkit(this))
    , mode(Toolkit::MoveMode)
    , deleteAction( new QAction(tr("删除细胞"), this))
    //, typesGroup( new QActionGroup(this))
{
    setDragMode(ScrollHandDrag);
    connect(tookit, &Toolkit::modeChanged, this, [this](int mode){
        if (mode == Toolkit::MoveMode && this->mode == Toolkit::CaptureMode)
        {
            this->mode = mode;
            update();

            auto center = viewport()->rect().center();
            QRect roi(center - QPoint(300, 225), center + QPoint(300, 225));

            QImage image(roi.size(), QImage::Format_RGB888);
            image.fill(Qt::transparent);
            QPainter painter(&image);

            auto items = scene()->items(mapToScene(roi));
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

            render(&painter, image.rect(), roi);
            std::for_each(items.begin(), items.end(), [](QGraphicsItem* item){
               item->show();
            });
            captured(QPixmap::fromImage(image));
        }
        else if (mode == Toolkit::MoveMode)
        {
            this->mode = mode;
            setDragMode(ScrollHandDrag);
            setInteractive(true);
            resetTransform();
            ViewportAnchor oldAnchor = transformationAnchor();
            setTransformationAnchor(QGraphicsView::NoAnchor);
            setTransform(saved);
            setTransformationAnchor(oldAnchor);
            centerOn(center);

        }
        else if (mode == Toolkit::EditMode)
        {
            this->mode = mode;
            saved = transform();
            center = mapToScene(viewport()->rect().center());

            setDragMode(NoDrag);
            setInteractive(true);
//            auto point = mapToScene(viewport()->rect().center());
            ViewportAnchor oldAnchor = transformationAnchor();
//            setTransformationAnchor(QGraphicsView::NoAnchor);
//            QTransform trans;
//            trans.translate(point.x(), point.y()).scale(0.75, 0.75).translate(-point.x(), -point.y());
//            setTransform(trans);
//            setTransformationAnchor(oldAnchor);
            setTransformationAnchor(QGraphicsView::AnchorViewCenter);
            setTransform( transform().scale(1 / saved.m11(), 1 / saved.m11()));
            setTransformationAnchor(oldAnchor);
        }
        else if (mode == Toolkit::CaptureMode)
        {
            this->mode = mode;
            update();
        }
        tookit->setRegion(mapToScene(viewport()->rect()).boundingRect().toRect());
    });



}

void ImageView::setThumbnail(const QPixmap& pixmap, qreal ratio)
{
    tookit->setPixmap(pixmap, ratio);
}

void ImageView::setCategoryMap(const QMap<int, CategoryItem> &categorys)
{
    this->categoryMap = categorys;
}

void ImageView::updateToolKitRegion()
{
    tookit->setRegion(mapToScene(viewport()->rect()).boundingRect().toRect());

}

void ImageView::zoomAt(const QPoint &centerPos, double factor)
{
    if (dragMode() == NoDrag)
    {
        return;
    }
    //QGraphicsView::AnchorUnderMouse uses ::centerOn() in it's implement, which must need scroll.
    //transformationAnchor() default is AnchorViewCenter, you need set NoAnchor while change transform,
    //and combine all transform change will work more effective
    QPointF targetScenePos = mapToScene(centerPos);
    ViewportAnchor oldAnchor = transformationAnchor();
    setTransformationAnchor(QGraphicsView::NoAnchor);

    QTransform matrix = transform();

    matrix.translate(targetScenePos.x(), targetScenePos.y())
            .scale(factor, factor)
            .translate(-targetScenePos.x(), -targetScenePos.y());

    if (matrix.m11() >= 1 || matrix.m11() < 0.025)
    {
        return;
    }

    setTransform(matrix);

    setTransformationAnchor(oldAnchor);
    tookit->setRegion(mapToScene(viewport()->rect()).boundingRect().toRect());
}

#if QT_CONFIG(wheelevent)
void ImageView::wheelEvent(QWheelEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {

        event->ignore();
        return;
    }

    double factor = qPow(1.001, event->angleDelta().y());
    zoomAt(event->pos(), factor);
    event->accept();
}
#endif

void ImageView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QGraphicsView::mousePressEvent(event);

    }
}

void ImageView::mouseReleaseEvent(QMouseEvent *event)
{
    if (dragMode() == ScrollHandDrag && !isInteractive())
    {
//        emit regionChanged(mapToScene(viewport()->rect()),matrix().m11());
    }
    if (DragMode::NoDrag == dragMode())
    {
        emit objectAdded(mapToScene(drawing));

//        QPainterPath path;
//        path.addPolygon(mapToScene(drawing));
//        path.setFillRule(Qt::WindingFill);
//        auto rect = path.toFillPolygon().boundingRect().toRect();
//        path.translate(-rect.topLeft());
//        QImage image(rect.size(), QImage::Format_RGB888);
//        image.fill(Qt::black);
//        QPainter painter(&image);
//        painter.setClipPath(path);
//        auto items = scene()->items(rect);
//        for(auto iter = items.begin(); iter != items.end();)
//        {
//            if (dynamic_cast<CellGraphicsItem*>(*iter) && (*iter)->isVisible())
//            {
//                (*iter++)->hide();
//            }
//            else
//            {
//                iter = items.erase(iter);
//            }
//        }

//        scene()->render(&painter, image.rect(), rect);
//        std::for_each(items.begin(), items.end(), [](QGraphicsItem* item){
//           item->show();
//        });
//        QPolygon polygon = findCell(image);
//        scene()->addPolygon(polygon.translated(rect.topLeft()));

        drawing.clear();
        viewport()->update();
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void ImageView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    if (DragMode::NoDrag == dragMode() && event->buttons() & Qt::LeftButton)
    {
        drawing.append(event->pos());
        viewport()->update();
    }
    if (DragMode::ScrollHandDrag  == dragMode() && event->buttons() & Qt::LeftButton)
    {
        tookit->setRegion(mapToScene(viewport()->rect()).boundingRect().toRect());
    }
}

void ImageView::mouseDoubleClickEvent(QMouseEvent *event)
{
    CellGraphicsItem* cell = qgraphicsitem_cast<CellGraphicsItem *>(this->itemAt(event->pos()));
    IF_RETURN(cell==nullptr);
    TRACE() << "double Click on cell" << cell->id();
    emit doubleClicked(cell->id());
}



void ImageView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    if (DragMode::NoDrag == dragMode() && !drawing.isEmpty())
    {
        QPen pen(Qt::red);
        pen.setWidth(int(3 / matrix().m11()));
        painter->setPen(pen);
        painter->drawPolyline(mapToScene(drawing));

        auto bounding = drawing.boundingRect();
        if (bounding.width() > 500 || bounding.height() > 500)
        {
            pen.setColor(Qt::cyan);
            painter->setPen(pen);
            painter->drawPolygon(mapToScene(bounding));
        }
    }
    if (mode == Toolkit::CaptureMode)
    {

//        QRegion region(center.to);
//           QRegion region2(polygon2.toPolygon());

//           QPainterPath painterPath;
//           painterPath.addRegion(region.subtracted(region2));
//           painter.fillPath(painterPath,Qt::red);

        QRegion region(viewport()->rect());
        auto center = viewport()->rect().center();
        QRect roi(center - QPoint(300, 225), center + QPoint(300, 225));

        QPainter p(viewport());
        p.save();
        QPainterPath painterPath;
        painterPath.addRegion(region.subtracted(roi));
        p.fillPath(painterPath, QBrush(QColor(64, 64, 64, 128)));
        p.setPen(Qt::white);
        p.drawRect(roi);
        p.restore();
    }

}


void ImageView::contextMenuEvent(QContextMenuEvent *event)
{
    //TRACE();
    // 判断鼠标位置上的Item, 如果是CellItem, 则处理. 否则不做任何处理.
    CellGraphicsItem* cell = qgraphicsitem_cast<CellGraphicsItem *>(this->itemAt(event->pos()));
    IF_RETURN(cell==nullptr);
    QActionGroup* typeGroup = new QActionGroup(this);
    for(const auto& item: cell->getItem().getCatalog())
    {
        auto action = new QAction(item.cnName);
        action->setChecked(true);
        action->setData(item.id);
        typeGroup->addAction(action);
    }
    typeGroup->setExclusive(true);

    // 删除细胞菜单项, 通过信号sigViewCellDeleted发给AnalyzeWindow
    // 注意， 这里仅仅发信号出去，不做任何修改。
    connect(this->deleteAction, &QAction::triggered, [&](){
        // 支持一次性删除多个细胞。 但是不支持修改类别的操作。
        emit sigCellDeleted(cell->id());
    });

    // 处理变更细胞类型的子菜单项目, 通过信号sigViewCellTypeChanged发给AnalyzeWindow处理
    // 注意, 这里仅仅是发送出去信号
    connect(typeGroup, &QActionGroup::triggered, this, [&](QAction* action){
        //TRACE() << "cell: " << cell->id() << cell->getCategoryId();
        int new_type = action->data().toInt();
        int cellid = cell->id();
        int old_type = cell->getCategoryId();
        // 注意, 不在这里修改, 而是在CellShowPage里面做实际的修改操作.
        emit sigCellTypeChanged(cellid, new_type, old_type);
    });

    // 创建右键菜单和子菜单
    QMenu contextMenu;
    contextMenu.addAction(deleteAction);


    QMenu typesMenu(tr("修改细胞类别"));
    contextMenu.addMenu(&typesMenu);
    // 检查当前细胞的类别.
    int categoryId = cell->getCategoryId();
    for(auto a: typeGroup->actions()){
        if( a->data()==categoryId){
            a->setChecked(true);
            break;
        }
    }

    typesMenu.addActions(typeGroup->actions());
    contextMenu.exec(QCursor::pos());

    // 要disconnect, 不然每次进去都绑定一次, 会导致不停重复执行.
    disconnect(this->deleteAction, &QAction::triggered, nullptr,nullptr);
    disconnect(typeGroup, &QActionGroup::triggered, nullptr, nullptr);

    qDeleteAll(typeGroup->actions());
    delete typeGroup;
}


