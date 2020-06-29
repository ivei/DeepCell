#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include "celllistmodel.h"
#include "../srv/dbgutility.h"
#include "../srv/cellitem.h"

#pragma execution_character_set("utf-8")

CellListModel::CellListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CellListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return cells.size();
}

QVariant CellListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::UserRole)
    {
        return QVariant::fromValue(cells.at(index.row()));
    }
    if (role == Qt::DecorationRole)
    {
        CellItem cell = cells.at(index.row());
        QPixmap pixmap = cell.pixmap();
        QString title = cell.title();
        QPainter painter(&pixmap);
        QFont font("Arial", 15);
        painter.setFont(font);
        QPen pen(Qt::red);
        painter.setPen(pen);
        painter.drawText(10,20, title);

        //return QIcon(cells.at(index.row()).pixmap());
        return QIcon(pixmap);
    }

    return QVariant();
}

bool CellListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    const auto it = cells.begin() + row;
    cells.erase(it, it + count);

    endRemoveRows();

    return true;
}
#include <QDebug>
bool CellListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
    {
        return false;
    }
    auto& item = cells[index.row()];
    switch (role)
    {
    case TYPE_ROLE:
        emit cellTypeChanged(item, value.value<int>());
        //item.value().category_id = value.value<int>();
        item.setCategoryId(value.value<int>());
        emit dataChanged(index, index, {Qt::EditRole, Qt::DecorationRole});

        break;
    case COUNTOURS_ROLE:
        item.value()._contours = value.value<QPolygon>();
        emit dataChanged(index, index, {Qt::DecorationRole});
        break;
    case STATUS_ROLE:
        emit cellRemoved(item);
        item.value()._deleted = true;
        emit dataChanged(index, index, {Qt::EditRole});
        break;
    }

    return true;
}

void CellListModel::setSource(const QList<CellItem>& cells)
{
    beginResetModel();
    this->cells = cells;
    endResetModel();
}

void CellListModel::setDelete(int id, bool deleted)
{
    int i = 0;
    for (auto& item: cells)
    {

        if (item.uid() == id)
        {
            if (deleted)
            {
                emit cellRemoved(item);
            }
            else
            {
                emit cellAdded(item);
            }
            //?
            item.value()._deleted = deleted;
            emit dataChanged(index(i), index(i), {Qt::EditRole});
            break;
        }
        ++i;
    }
}

void CellListModel::changeType(int id, int type)
{
    int i = 0;
    for (auto& item: cells)
    {

        if (item.uid() == id)
        {
            emit cellTypeChanged(item, type);
            item.value()._category_id = type;
            emit dataChanged(index(i), index(i), {Qt::EditRole});
            break;
        }
        ++i;
    }
}

void CellListModel::add(CellItem& item)
{
    TRACE();
    auto iter = std::find_if(cells.begin(), cells.end(), [id=item.uid()](const CellItem& c) {
        return id == c.uid();
    });

    if (iter == cells.end())
    {
        TRACE() << "cell is not a deleted cell";
        auto row = rowCount();
        item.setObjectId(-row);
        item.setUid(-row);
        item.value()._deleted = false;
        TRACE() << item.toString();
        beginInsertRows(QModelIndex(), row, row);
        cells.push_back(item);
        endInsertRows();
        TRACE() << "added";
        emit cellAdded(item);
    }
    else if (iter->value()._deleted)
    {
        TRACE() << "cell is a before deleted cell. ";
        iter->value()._deleted = false;
        emit cellAdded(item);
        auto i = int(std::distance(cells.begin(), iter));
        emit dataChanged(index(i), index(i), {Qt::EditRole});

    }
}

void CellListModel::sync(const QList<CellItem>& newCells)
{
    TRACE();
    for (auto iter = cells.begin(); iter != cells.end();)
    {
        if (iter->deleted() || !iter->isValid())
        {
            auto row = int(std::distance(cells.begin(), iter));
            beginRemoveRows(QModelIndex(), row, row);
            auto cell = *iter;
            iter = cells.erase(iter);
            endRemoveRows();
            emit cellRemoved(cell);

        }
        else
        {
            iter->value()._oldType = iter->value()._category_id;
            ++iter;
        }
    }

    if( newCells.isEmpty()){
        return;
    }
    auto row = rowCount();
    beginInsertRows(QModelIndex(), row, row + newCells.count() - 1);
    cells.append(newCells);
    endInsertRows();
    for (auto& item: newCells)
    {
        emit cellAdded(item);
    }
}

void CellListModel::onSigShowCellDeleted(int cellId)
{
    TRACE() << "to delete cell id=" << cellId << "from model";
    auto iter = std::find_if(this->cells.begin(), this->cells.end(), [cellId](const CellItem& c){ return c.uid()==cellId;});
    if( iter==cells.end()){
        // 没有找到
        TRACE() << "Not found!";
    }
    else {
        TRACE() << "Delete cell" << cellId;
        auto row = int(std::distance(cells.begin(), iter));
        beginRemoveRows(QModelIndex(), row, row);
        iter = cells.erase(iter);
        endRemoveRows();
    }
}

void CellListModel::onSigShowTypeChanged(int cellId, int newtype, int oldtype)
{
    TRACE() << QString("to modify cell%1 type from %2 to %3 in model").arg(cellId).arg(oldtype).arg(newtype);
    auto iter = std::find_if(cells.begin(), cells.end(), [cellId](const CellItem& c){ return c.uid() == cellId; });
    if( iter == cells.end()){
        TRACE() << "Can not find cell id=" << cellId << "in model!";
        return;
    }
    else {
        TRACE() << "find cell. to modify type...";
        auto idx = static_cast<int>(std::distance(cells.begin(), iter));

        iter->setCategoryId(newtype);
        emit dataChanged(index(idx), index(idx), {Qt::EditRole});
    }
}

QList<CellItem> &CellListModel::getCells()
{
    return this->cells;
}

int CellListModel::validCount() const
{
    return std::count_if(this->cells.begin(), this->cells.end(),
                  [](const CellItem& a){return a.deleted() == false;});
}



#if 0
void CellListModel::selectItem(int cellId)
{
    TRACE() << QString("to select cell%1 ").arg(cellId);
    auto iter = std::find_if(cells.begin(), cells.end(), [cellId](const CellItem& c){ return c.id() == cellId; });
    if( iter == cells.end()){
        TRACE() << "Can not find cell id=" << cellId << "in model!";
        return;
    }
    else {

    }
}
#endif


