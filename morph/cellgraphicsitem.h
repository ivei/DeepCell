#ifndef CELLGRAPHICSITEM_H
#define CELLGRAPHICSITEM_H
#include <QGraphicsPolygonItem>

#include "../srv/cellitem.h"
#include "../srv/category.h"

class DeepLabel;

/**
 * @brief 用于在ImageView中显示的细胞(本质上是轮廓)
 */
class CellGraphicsItem : public QObject, public QGraphicsPolygonItem
{
    Q_OBJECT
    //Q_INTERFACES(QGraphicsPolygonItem)
public:
    enum { Type = UserType + 1 };
    int type() const override
    {
        // Enable the use of qgraphicsitem_cast with this item.
        return Type;
    }
    CellGraphicsItem(const CellItem& item, DeepLabel* handler/*, const QMap<int, CategoryItem>& categorys*/, bool showMark, bool showCounter);
    void initialize();
    //! 返回里面保存的CellItem的信息
    CellItem getItem() const;
    //! 设置类型ID.
    void setCategoryId(int categoryId);
    //! 返回细胞的分类ID
    int getCategoryId() const;

    //! 返回细胞编号
    inline int id() const { return item.uid(); }
    inline const QString titleText() const { return this->item.title() ; }
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setShowMark(bool isShow);
    void setShowCounter(bool isShow);


protected:
    // 上下文菜单改到ImageView中进行
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
signals:
    void sigCellDelete(const CellItem& item);
    void sigCellTypeChanged(const CellItem& item, int new_type);

private slots:
    void onDownloadFinished();

protected:
    void setColor(int type);
    QColor getPenColor(int type);
    QColor getFontColor(int type);
private:
    CellItem item;
    //QMap<int, CategoryItem> _categorys; //! 细胞类别字典
    bool   _showMark;       //! 绘制控制参数, 控制是否绘制细胞的方框和外边界
    bool   _showCounter;    //! 绘制控制参数, 控制是否绘制细胞的轮廓
    DeepLabel *_handler;
};

#endif // CELLGRAPHICSITEM_H
