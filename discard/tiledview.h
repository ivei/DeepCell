#ifndef TILEDVIEW_H
#define TILEDVIEW_H
#include "imageitem.h"
#include <QWidget>
#include <QMap>

namespace Ui {
class TiledView;
}
class CategoryItem;
class TaskResult;
class CellItem;
class QGraphicsItem;
class DeepLabel;
class ImageView;
class TiledView : public QWidget
{
    Q_OBJECT

public:
    explicit TiledView(QWidget *parent = nullptr);
    ~TiledView();

    void initialize(const TaskResult &result, DeepLabel* handler);
    QImage capture(const QRect& rect);
    ImageView *view();
    inline const ImageItem& currentImage() const { return image; }

    bool cellMarkShow() const;
    bool cellCounterShow() const;

public Q_SLOTS:
    void onCellAdded(const CellItem& item);
    void onCellSelected(const CellItem&);
    void onCellDeleted(const CellItem&);
    void onCellTypeChanged(const CellItem&, int type);
    void onCellShowMarkChanged(bool isShow);
    void onCellShowCounterChanged(bool);
private:
    Ui::TiledView *ui;
    QMap<int, QGraphicsItem*> items;
    ImageItem image;
    QMap<int, CategoryItem> categorys;
    bool _cellMarkShow;
    bool _cellCounterShow;
};

#endif // TILEDVIEW_H
