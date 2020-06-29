#ifndef GALLERYVIEW_H
#define GALLERYVIEW_H
#include <QWidget>
#include <QMap>
#include "imageitem.h"

namespace Ui {
class GalleryView;
}
class CategoryItem;
class QGraphicsItem;
class TaskResult;
class CellItem;
class DeepLabel;
class ImageView;

class GalleryView : public QWidget
{
    Q_OBJECT

public:
    explicit GalleryView(QWidget *parent = nullptr);
    ~GalleryView();

    void initialize(const TaskResult& result, DeepLabel *provider);
    QImage capture(const QRect& rect);
    ImageView *view();
    inline const ImageItem& currentImage() const { return image; }

    bool cellMarkShow() const;
    bool cellCounterShow() const;

public Q_SLOTS:
    void onCellAdded(const CellItem&);
    void onCellSelected(const CellItem&);
    void onCellDeleted(const CellItem&);
    void onCellTypeChanged(const CellItem&, int type);
    void onCellShowMarkChanged(bool);
    void onCellShowCounterChanged(bool);
private:
    Ui::GalleryView *ui;
    QMap<int, QGraphicsItem*> items;
    ImageItem image;
    QMap<int, CategoryItem> categorys;
    bool _cellMarkShow;
    bool _cellCounterShow;


    QMap<QString, QGraphicsItem*> images;
    QMap<QString, ImageItem> imageItems;
};

#endif // GALLERYVIEW_H
