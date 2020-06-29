#ifndef CELLITEMDELETATE_H
#define CELLITEMDELETATE_H
#include <QStyledItemDelegate>
#include <QList>

class DeepLabel;
class CellItemDeletate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    CellItemDeletate(DeepLabel *provider, QObject *parent = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index) const override;

protected slots:
    void download(const QString& url, const QModelIndex&);
    void onReplyFinised(const QString& url);

protected:
    void initStyleOption(QStyleOptionViewItem *option,
                    const QModelIndex &index) const override;
private:
    DeepLabel *provider;
    QMap<QString, QPixmap> cached;
    QMap<QString, QList<QModelIndex>> pending;
};

#endif // CELLITEMDELETATE_H
