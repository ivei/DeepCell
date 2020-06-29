#ifndef TASKPROXYMODEL_H
#define TASKPROXYMODEL_H

#include <QList>
#include <QSet>
#include <QSortFilterProxyModel>

#include "../srv/commdefine.h"
class TaskProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum TProxyRefreshPolicy{
        AlwaysRefresh = 0,
        NotRefresh,
        SmartRefresh
    };
    TaskProxyModel(QObject *parent = nullptr, TProxyRefreshPolicy refresh_policy=TProxyRefreshPolicy::AlwaysRefresh);
    bool setFilters(
              QSet<int> assays={}
            , QSet<int> my_status={}
            , QSet<QString> case_numbers = {}
            , QSet<int> device_ids = {}
            , QSet<int> slide_status = {}
            );
    void clearFilters(bool is_refresh=false);
    bool setAssayTypeRule(const QSet<int>& rules={}, bool force_refresh=false);
    bool setMyStatusRule(const QSet<int>& rules={}, bool force_refresh=false);
    bool setCaseNumberRule(const QSet<QString>& rules={}, bool force_refresh=false);
    bool setDeviceIdRule(const QSet<int>& rules={}, bool force_refresh=false);
    bool setSlideStatusRule(const QSet<int>& rules={}, bool force_refresh=false);
    void refresh(bool force_refresh=true);
    void setRefreshPolicy(TProxyRefreshPolicy policy){ this->_refresh_policy = policy; }
    void copyFilters(const TaskProxyModel* ptr);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
private:
    QSet<int>   _rule_assay_type_id; // 玻片ID
    QSet<int>   _rule_my_status;
    QSet<int>   _rule_slide_status;
    QSet<int>   _rule_device_id;
    QSet<QString>  _rule_case_number;
    TProxyRefreshPolicy _refresh_policy;

};

#endif // TASKPROXYMODEL_H
