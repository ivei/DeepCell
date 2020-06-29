#ifndef CELLLISTTYPEFILTERMODEL_H
#define CELLLISTTYPEFILTERMODEL_H

#include <QSortFilterProxyModel>

class CellListTypeFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit CellListTypeFilterModel(QObject *parent = nullptr);
    void setFilter(int t) { type = t; }
    inline int filter() const { return type; }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    int type;
};

#endif // CELLLISTTYPEFILTERMODEL_H
