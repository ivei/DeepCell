#include "toolkit.h"
#include "ui_toolkit.h"
#include <QStyle>
#include <QGraphicsPixmapItem>
#include "../srv/dbgutility.h"

#pragma execution_character_set("utf-8")

Toolkit::Toolkit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Toolkit),
    ratio(0),
    item(nullptr),
    pix(nullptr)

{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground, true);  // FramelessWindowHint & WA_TranslucentBackground， make transperent dialog
    setStyleSheet("background:transparent;color:white;");
    ui->graphicsView->setScene(new QGraphicsScene(this));
    connect(ui->mode, &QToolButton::clicked, this, [this](bool checked){
       ui->thumbnail->setVisible(!checked);
       ui->capture->setVisible(!checked);
       emit this->modeChanged(checked ? EditMode: MoveMode);
    });
    connect(ui->graphicsView->scene(), &QGraphicsScene::sceneRectChanged, this, [this](const QRectF& rect){
        ui->graphicsView->fitInView(rect, Qt::KeepAspectRatio);
    });
    connect(ui->capture, &QToolButton::clicked, this, [this](bool checked){
        ui->thumbnail->setVisible(!checked);
        ui->mode->setVisible(!checked);
        emit this->modeChanged(checked ? CaptureMode: MoveMode);
    });
}

Toolkit::~Toolkit()
{
    delete ui;
}

void Toolkit::setPixmap(const QPixmap& pixmap, qreal ratio)
{
#if 0
    TRACE() << "begin set Pixmap";
    auto scene = ui->graphicsView->scene();
    scene->clear();
    //qDeleteAll(items);

    scene->addPixmap(pixmap);
    this->ratio = ratio;
    this->region = pixmap.rect();
    TRACE() << "end set pixmap";
#else
    if( this->pix == nullptr){
        this->pix = ui->graphicsView->scene()->addPixmap(pixmap);
    }
    else {
        this->pix->setPixmap(pixmap);
    }
    this->ratio = ratio;
    this->region = pixmap.rect();
#endif
}


void Toolkit::setRegion(const QRect& rect)
{
#if 0
    if (item)
    {
        TRACE() << "Delete Item...";
        ui->graphicsView->scene()->removeItem(item);
        //delete item;
    }
    auto roi = region.intersected(QRect(rect.topLeft() * ratio, rect.bottomRight() * ratio));
    item = ui->graphicsView->scene()->addRect(roi, QPen(Qt::red));
#else
    auto roi = region.intersected(QRect(rect.topLeft() * ratio, rect.bottomRight() * ratio));
    if( this->item == nullptr)
    {
        this->item = ui->graphicsView->scene()->addRect(roi, QPen(Qt::red));
    }
    else{
        this->item->setRect(roi);
    }
#endif

}
