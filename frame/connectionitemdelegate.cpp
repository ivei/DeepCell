#include "connectionitemdelegate.h"
#include "ui_connectionitem.h"
#include "connectionitem.h"
#include <QWidget>
#include <QPainter>
#include "../srv/dbgutility.h"

#pragma execution_character_set("utf-8")

ConnectionItemDelegate::ConnectionItemDelegate(QObject *parent): QStyledItemDelegate(parent),delegateWidget(new QWidget), ui(new Ui::ConnectionItem)
{
    ui->setupUi(delegateWidget.get());
}

ConnectionItemDelegate::~ConnectionItemDelegate()
{

}
#include <QDebug>

void ConnectionItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{

    if (index.data(Qt::UserRole).canConvert<ConnectionItem>()) {
        ConnectionItem item = qvariant_cast<ConnectionItem>(index.data(Qt::UserRole));

        if (option.state & QStyle::State_Selected)
            delegateWidget->setStyleSheet("background-color: rgb(33, 34, 34);color: rgb(208, 208, 208);");
        else if(option.state & QStyle::State_MouseOver)
            delegateWidget->setStyleSheet("background-color: rgb(94, 95, 96);color: rgb(208, 208, 208);");
        else
            delegateWidget->setStyleSheet("background-color: rgb(64, 65, 66);color: rgb(208, 208, 208);");

        ui->name->setText(item.profileName);
        ui->userTextLabel->setText(item.username);
        ui->hostTextLabel->setText(QString("%1:%2").arg( item.domainName).arg(item.port));
        painter->save();
        painter->translate(option.rect.topLeft());
        delegateWidget->render(painter, QPoint(), QRegion(), QWidget::DrawChildren);
        painter->restore();

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ConnectionItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    //TRACE() << delegateWidget->minimumSizeHint() << delegateWidget->minimumSize() << endl;
    return delegateWidget->minimumSize();
}
