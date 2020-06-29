#include "../srv/cellitem.h"

#include "celllisttypefiltermodel.h"

#pragma execution_character_set("utf-8")


CellListTypeFilterModel::CellListTypeFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool CellListTypeFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    auto item = index.data(Qt::UserRole).value<CellItem>().value();
    return item._category_id == type && !item._deleted;
}


