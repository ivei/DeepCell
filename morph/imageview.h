#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QAction>
#include <QActionGroup>

class Toolkit;
class CategoryItem;
class CellItem;
class ImageView : public  QGraphicsView
{
    Q_OBJECT
public:
    ImageView(QWidget *parent = nullptr);

    void setThumbnail(const QPixmap& pixmap, qreal ratio);
    void setCategoryMap(const QMap<int, CategoryItem>& categorys);
    void updateToolKitRegion();
    void setCellFamily(const QString& family){ this->family=family; }
    QString getCellFamily() const { return this->family; }
Q_SIGNALS:
    void objectAdded(const QPolygonF& polygon);
    void captured(const QPixmap& pixmap);

    //! 细胞删除
    void sigCellDeleted(int cellId );
    //! 修改了细胞类别
    void sigCellTypeChanged(int cellId, int newTypeId, int oldTypeId);

    //! 用户双击了细胞
    void doubleClicked(int cellId);
protected:
    void zoomAt(const QPoint &centerPos, double factor);

#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *) override;
#endif

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Toolkit *tookit;    // 显示缩略图的小窗口
    QPolygon drawing;
    QTransform saved;
    QPointF center;
    int mode;   //! 浏览模式, 编辑模式, 抓图模式..
    QMap<int, CategoryItem> categoryMap;    //  类别定义.
    QString     family;     //! 当前显示的视野的细胞族
    QAction *deleteAction;
    //QActionGroup *typesGroup;
};

#endif // IMAGEVIEW_H
