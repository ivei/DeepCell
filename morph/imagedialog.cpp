#include "imagedialog.h"
#include "ui_imagedialog.h"

#pragma execution_character_set("utf-8")

ImageDialog::ImageDialog(const QList<QPixmap>& pixmaps, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageDialog)
{
    ui->setupUi(this);
    for (auto pixmap: pixmaps)
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap), QString(), ui->listWidget);
        ui->listWidget->addItem(item);
    }
}

ImageDialog::~ImageDialog()
{
    delete ui;
}
