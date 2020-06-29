#ifndef CONNECTIONITEMDELEGATE_H
#define CONNECTIONITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QScopedPointer>
class QWidget;

namespace Ui {
    class ConnectionItem;
}

class ConnectionItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ConnectionItemDelegate(QObject *parent = nullptr);
    ~ConnectionItemDelegate() override;

//    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QScopedPointer<QWidget> delegateWidget;
    QScopedPointer<Ui::ConnectionItem> ui;
};

#endif // CONNECTIONITEMDELEGATE_H
