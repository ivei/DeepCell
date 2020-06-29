#ifndef TOOLKIT_H
#define TOOLKIT_H

#include <QWidget>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>

namespace Ui {
class Toolkit;
}
class QGraphicsScene;
class QGraphicsItem;
class Toolkit : public QWidget
{
    Q_OBJECT

public:
    enum Mode
    {
        MoveMode,
        EditMode,
        CaptureMode,
    };
    explicit Toolkit(QWidget *parent = nullptr);
    ~Toolkit();

    void setPixmap(const QPixmap& pixmap, qreal ratio);
    void setRegion(const QRect& rect);
signals:
    void centerOn(const QPointF& pos);
    void modeChanged(int type);
private:
    Ui::Toolkit *ui;
    qreal ratio;
    QRect region;
    #if 0
    QGraphicsItem *item;
#else
    QGraphicsRectItem *item;
    QGraphicsPixmapItem *pix;

#endif
};

#endif // TOOLKIT_H
