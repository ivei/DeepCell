#ifndef UNASSIGNEDTASKLISTMODEL_H
#define UNASSIGNEDTASKLISTMODEL_H

#include <QAbstractTableModel>
#include <QTimer>
#include "../srv/commdefine.h"
#include "../srv/datalist.h"
#include "batchmodel.h"

class SlideItem;
class DeepLabel;
class UnassignedTaskListModel : public BatchModel
{
    Q_OBJECT

public:
    explicit UnassignedTaskListModel(QObject *parent = nullptr, int interval=0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;


    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // custom functionanlity
    void setProvider(DeepLabel *provder) { this->provider = provder; }
protected:
#if 0
    // Fetch data dynamically:
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
#endif
    bool _hasMore() override;
    void _fetchOnce() override;

signals:
    void fetchDone() const;
private:
    DataList<SlideItem> tasks;
    DeepLabel *provider;
    QTimer _fetchTimer;     // 间隔定时器
    int    _interval;       // 两次查询的间隔时间
//    int   _assay_type_id;
//    QString      _case_number;
//    int          _device_id;

};

#endif // UNASSIGNEDTASKLISTMODEL_H
