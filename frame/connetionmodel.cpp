#include "connetionmodel.h"
#include "connectionitem.h"

#pragma execution_character_set("utf-8")

ConnetionModel::ConnetionModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ConnetionModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return items.count();
}

QVariant ConnetionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::UserRole)
    {
        return QVariant::fromValue(items.at(index.row()));
    }

    return QVariant();
}

bool ConnetionModel::addItem(const ConnectionItem& item)
{
    auto row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    items.append(item);
    endInsertRows();
    return true;
}

bool ConnetionModel::removeItem(int row)
{
    if (row < 0 || row >= rowCount())
    {
        return false;
    }
    beginRemoveRows(QModelIndex(), row, row);
    items.removeAt(row);
    endRemoveRows();
    return true;
}

bool ConnetionModel::modifyItem(int row, const ConnectionItem& item)
{
    if (row < 0 || row >= rowCount())
    {
        return false;
    }
    items.replace(row, item);
    emit dataChanged(index(row), index(row), {Qt::DisplayRole});
    return true;
}

void ConnetionModel::setItems(const QList<ConnectionItem>& items)
{
    beginResetModel();
    this->items.clear();
    this->items.append(items);
    endResetModel();
}

