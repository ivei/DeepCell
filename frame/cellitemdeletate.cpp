#include <QPainter>
#include <QDebug>

#include "../srv/deeplabel.h"
#include "../srv/deeplabelreply.h"
#include "../srv/cellitem.h"

#include "cellitemdeletate.h"
#pragma execution_character_set("utf-8")


CellItemDeletate::CellItemDeletate(DeepLabel *provider, QObject *parent)
    : QStyledItemDelegate(parent),
      provider(provider)
{

}

void CellItemDeletate::paint(QPainter *painter,
           const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(Qt::UserRole).canConvert<CellItem>()) {
        auto item = qvariant_cast<CellItem>(index.data(Qt::UserRole));
        auto url = item.url();
        if (cached.contains(url))
        {
            painter->drawPixmap(option.rect, cached[url]);
        }
        else
        {
            const_cast<CellItemDeletate*>(this)->download(url, index);
        }
    }

    QStyledItemDelegate::paint(painter,  option, index);
}

QSize CellItemDeletate::sizeHint(const QStyleOptionViewItem &option,
                const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(250, 250);
}

void CellItemDeletate::initStyleOption(QStyleOptionViewItem *option,
                const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}

void CellItemDeletate::download(const QString& url, const QModelIndex& index)
{
    if (pending.contains(url))
    {
        return;
    }
    pending[url].append(index);
    if (pending[url].count() > 1)
    {
        return;
    }

    auto reply = provider->download(url);
    connect(reply, &DeepLabelReply::finished, this, std::bind(&CellItemDeletate::onReplyFinised, this, url));
}

void CellItemDeletate::onReplyFinised(const QString& url)
{
    DeepLabelReply* reply = qobject_cast<DeepLabelReply *>(sender());
    auto it = pending.find(url);
    if (it == pending.end())
    {
        reply->deleteLater();
        return;
    }

    if (reply->error() != DeepLabelReply::NoError)
    {
        pending.erase(it);
        reply->deleteLater();
        return;
    }

    QPixmap pixmap;
    pixmap.loadFromData(reply->rawData());
    cached.insert(url, pixmap);
    for (auto req : it.value())
    {
        const_cast<QAbstractItemModel*>(req.model())->dataChanged(req, req, {Qt::DecorationRole});
    }
    pending.erase(it);
    reply->deleteLater();
}
