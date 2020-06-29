#ifndef TASKLISTMODEL_H
#define TASKLISTMODEL_H

#include "../srv/datalist.h"
#include <QAbstractTableModel>
#include <QTimer>
#include "../srv/commdefine.h"
#include "batchmodel.h"
class SlideItem;
class DeepLabel;

/**
 * @brief 实现玻片列表管理的数据模型类(我的任务)
 */
class TaskListModel : public BatchModel
{
    Q_OBJECT

public:

    explicit TaskListModel(QObject *parent = nullptr, int interval=0
                           //, int my_status=TSlideStatus::SLIDE_DOING
                           //, int assay_type_id = TAssayType::ALL_TYPE
                           //, int device_id = 0
    );
    ~TaskListModel() override;
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setProvider(DeepLabel *provder) { this->provider = provder; }
#if 0
    //! 一次性下载所有数据
    void fetchAll();
    //! 启动自动分批下载
    void startAutoFetch();
#endif
protected:
    //! 判断是否还有更多的数据, 就是canFetchMore的行为
    virtual bool _hasMore();
    //! 下载一次, 就是fetchMore的行为.
    virtual void _fetchOnce();

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

signals:
    void fetchError(const QString& errString);
    void fetchProgress(int total, int current);

private:
    DataList<SlideItem> tasks;
    DeepLabel *provider;
    QTimer _fetchTimer;     // 间隔定时器
    int    _interval;       // 两次查询的间隔时间
    //int _assay_type_id;
    //int _my_status;
    //int     _device_id;
};

#endif // TASKLISTMODEL_H
