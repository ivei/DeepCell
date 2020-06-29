#ifndef CONNETIONMODEL_H
#define CONNETIONMODEL_H

#include <QAbstractListModel>
#include <QList>

class ConnectionItem;

class ConnetionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ConnetionModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool addItem(const ConnectionItem& item);
    bool removeItem(int row);
    bool modifyItem(int row, const ConnectionItem& item);
    void setItems(const QList<ConnectionItem>& items);
    inline const QList<ConnectionItem>& getItems() const { return items; }
private:
    QList<ConnectionItem> items;
};

#endif // CONNETIONMODEL_H
