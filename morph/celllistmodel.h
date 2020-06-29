#ifndef CELLLISTMODEL_H
#define CELLLISTMODEL_H
#include <QAbstractListModel>

class CellItem;
class CellListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        TYPE_ROLE = Qt::UserRole + 1,
        COUNTOURS_ROLE,
        STATUS_ROLE
    };
    explicit CellListModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void setSource(const QList<CellItem>& cells);

    inline const QList<CellItem>& getSource() const { return cells; }

    void setDelete(int id, bool deleted);

    void changeType(int id, int type);

    void add(CellItem& item);
    void sync(const QList<CellItem>& cells);

    //
    void onSigShowCellDeleted(int cellId);
    void onSigShowTypeChanged(int cellId, int newtype, int oldtype);
    //void selectItem(int cellId);

    QList<CellItem>& getCells();

    int validCount() const;

Q_SIGNALS:
    void cellAdded(const CellItem& item);
    void cellRemoved(const CellItem& item);
    void cellTypeChanged(const CellItem& item, int type);
private:
    QList<CellItem> cells;
};

#endif // CELLLISTMODEL_H
