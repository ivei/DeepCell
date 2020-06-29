#include "taskproxymodel.h"
#include <QDebug>
#include <QVariant>
#include <QDateTime>
#include "../srv/dbgutility.h"

#pragma execution_character_set("utf-8")

TaskProxyModel::TaskProxyModel(QObject *parent, TProxyRefreshPolicy refresh_policy)
    : QSortFilterProxyModel (parent)
    , _refresh_policy(refresh_policy)
{
    this->setDynamicSortFilter(true);
}

/**
 * @brief 一次性刷新所有的筛选条件, 并根据配置的刷新策略和筛选条件的变化来决定是否刷新
 * @param assays
 * @param my_status
 * @param case_numbers
 * @param device_ids
 * @param slide_status
 * @return 是否筛选条件发生了变化.
 * @note
 *      是否重新刷新ProxyModel取决于筛选条件的变化和全局的刷新策略.
 *      - 若刷新策略为AlwaysRefresh, 则刷新一次
 *      - 若刷新策略为NotRefresh, 则不刷新
 *      - 若刷新策略为SmartRefresh, 则只有在至少一个条件发生变化后才会刷新.若所有条件都没有变化,
 *        就不刷新.
 * @note
 *      判断变化的原则是和类中的属性进行比较. 因此, 用户自己保证完整性.
 */
bool TaskProxyModel::setFilters(
          QSet<int> assays
        , QSet<int> my_status
        , QSet<QString> case_numbers
        , QSet<int> device_ids
        , QSet<int> slide_status
        )
{
    bool isFilterChanged = false;
    isFilterChanged |= this->setAssayTypeRule(assays, false);
    isFilterChanged |= this->setMyStatusRule(my_status, false);
    isFilterChanged |= this->setCaseNumberRule(case_numbers, false);
    isFilterChanged |= this->setDeviceIdRule(device_ids, false);
    isFilterChanged |= this->setSlideStatusRule(slide_status, false);

    if( this->_refresh_policy==TProxyRefreshPolicy::AlwaysRefresh
            || (this->_refresh_policy==TProxyRefreshPolicy::SmartRefresh && isFilterChanged==true)){
        this->invalidateFilter();
    }
    return isFilterChanged;
}

/**
 * @brief 清除所有的筛选条件
 * @param force_refresh
 * @note
 *      清空所有的筛选条件
 *      由force_refresh来决定是否刷新.
 * @note
 *      注意, 这个函数和setFilters()函数之间不能同时使用, 尤其是,
 *      比如, 当先调用clearFilters(false), 然后调用setFilters(..),
 *      若全局刷新策略不是AlwaysRefresh, 则可能会错误地不刷新!
 *      一般清空下, 这个函数最好不要使用.
 */
void TaskProxyModel::clearFilters(bool force_refresh)
{
    this->_rule_assay_type_id.clear();
    this->_rule_my_status.clear();
    this->_rule_case_number.clear();
    this->_rule_slide_status.clear();
    this->_rule_device_id.clear();
    if( force_refresh ){
        this->invalidateFilter();
    }

}

/**
 * @brief 设置对AssayTypeId的过滤条件
 * @param rules
 * @param force 是否强制刷新. 当为true时, 只要改变就刷新;
 * @return 如果条件发生了变化, 返回true; 否则返回false
 */
bool TaskProxyModel::setAssayTypeRule(const QSet<int> &rules, bool force_refresh /*= false*/)
{
    if( this->_rule_assay_type_id != rules){
        this->_rule_assay_type_id = rules;
        refresh(force_refresh);
        return true;
    }
    return false;
}

bool TaskProxyModel::setMyStatusRule(const QSet<int> &rules, bool force_refresh /*= false*/)
{
    if( this->_rule_my_status != rules){
        this->_rule_my_status = rules;
        refresh(force_refresh);
        return true;
    }
    return false;
}

bool TaskProxyModel::setCaseNumberRule(const QSet<QString> &rules, bool force /*= false*/)
{
    if( this->_rule_case_number != rules){
        this->_rule_case_number = rules;
        refresh(force);
        return true;
    }
    return false;
}

bool TaskProxyModel::setDeviceIdRule(const QSet<int> &rules, bool force /*= false*/)
{
    if( this->_rule_device_id != rules){
        this->_rule_device_id = rules;
        refresh(force);
        return  true;
    }
    return false;
}

bool TaskProxyModel::setSlideStatusRule(const QSet<int> &rules, bool force)
{
    if( this->_rule_slide_status != rules ){
        this->_rule_slide_status = rules;
        refresh(force);
        return true;
    }
    return false;
}

/**
 * @brief 更新ProxyModel. 当force为true时, 强制更新.
 * @param force
 */
void TaskProxyModel::refresh(bool force)
{
    if( force){
        this->invalidateFilter();
    }
}

void TaskProxyModel::copyFilters(const TaskProxyModel *ptr)
{
    this->_rule_assay_type_id = ptr->_rule_assay_type_id;
    this->_rule_my_status = ptr->_rule_my_status;
    this->_rule_case_number = ptr->_rule_case_number;
    this->_rule_device_id  = ptr->_rule_device_id;
    this->_rule_slide_status = ptr->_rule_slide_status;
}

bool TaskProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    //TRACE() << QString("sourceRow=%1").arg(sourceRow);
    bool isAccept=true;
    QModelIndex index_mystatus = sourceModel()->index(sourceRow, TTaskViewColumnID::MyStatus, sourceParent);
    QModelIndex index_assay = sourceModel()->index(sourceRow, TTaskViewColumnID::AssayTypeId, sourceParent);
    QModelIndex index_case = sourceModel()->index(sourceRow, TTaskViewColumnID::CaseNumber, sourceParent);

#if 0   // 调试信息
    bool my_state_filter_result = this->_rule_my_status.empty() || this->_rule_my_status.contains(sourceModel()->data(index_mystatus).toInt());
    bool assay_id_filter_result = this->_rule_assay_type_id.empty() || this->_rule_assay_type_id.contains(sourceModel()->data(index_assay).toInt());
    bool case_num_filter_result = this->_rule_case_number.empty() || this->_rule_case_number.contains(sourceModel()->data(index_case).toString());
    TRACE() << QString("my_status filter: %1, assay_id_filter: %2, case_number_filter: %3, total: %4")
               .arg(my_state_filter_result).arg(assay_id_filter_result).arg(case_num_filter_result)
               .arg(my_state_filter_result && assay_id_filter_result && case_num_filter_result);
#endif
    isAccept =  (this->_rule_my_status.empty() || this->_rule_my_status.contains(sourceModel()->data(index_mystatus).toInt()))
             && (this->_rule_assay_type_id.empty() || this->_rule_assay_type_id.contains(sourceModel()->data(index_assay).toInt() ))
             //&& (this->_rule_case_number.empty() || this->_rule_case_number.contains(sourceModel()->data(index_case).toString()))
             && (this->_rule_case_number.empty() || sourceModel()->data(index_case).toString().contains( this->_rule_case_number.toList().first() )  )
            ;


    return isAccept;
}

/**
 * @brief 实现自定义的Item比较函数, 用于排序处理.
 * @param left
 * @param right
 * @return
 */
bool TaskProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    //TRACE() << QString("left.row is: %1, right.row is: %2").arg(left.row()).arg(right.row());
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);
    if( leftData.type() == QVariant::DateTime){
        return leftData.toDateTime() < rightData.toDateTime();
    }
    else if (leftData.type() == QVariant::Int) {
        return leftData.toInt() < rightData.toInt();
    }
    else //if (leftData.type() == QVariant::String)
    {
        return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
    }

}
