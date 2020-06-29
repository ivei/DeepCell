
#include "../srv/commdefine.h"
#include "../srv/dbgutility.h"
#include "../srv/deeplabel.h"
#include "../srv/slideitem.h"

#include "unassignedtasklistmodel.h"
#pragma execution_character_set("utf-8")

//static QStringList headers = {/*"UID",*/ "ID", "Type", "Count"/*, "Status"*/};

UnassignedTaskListModel::UnassignedTaskListModel(QObject *parent
         , int interval
         )
    : BatchModel (interval, parent)
    //: QAbstractTableModel(parent)
    , provider(nullptr)
    , _interval(interval)
{
    connect(&this->_fetchTimer, &QTimer::timeout, [this](){this->fetchMore(QModelIndex());});
}



QVariant UnassignedTaskListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section) {
        case TTaskViewColumnID::UID:
            return tr("UID");
        case TTaskViewColumnID::SampleId:
            return tr("样本号");
        case TTaskViewColumnID::AssayType:
            return tr("样本类型");
        case TTaskViewColumnID::CaseNumber:
            return tr("病历号");
        case TTaskViewColumnID::CellCount:
            return tr("细胞数");
        case TTaskViewColumnID::ViewCount:
            return tr("视野数");
        case TTaskViewColumnID::CreateTime:
            return tr("创建时间");
        case TTaskViewColumnID::AssayTypeId:
            return  tr("样本类型编号");
        case TTaskViewColumnID::DeviceType:
            return tr("扫描仪");
        case TTaskViewColumnID::DeviceTypeId:
            return tr("扫描仪ID");
        case TTaskViewColumnID::MyStatus:
            return tr("我的任务状态");
        case TTaskViewColumnID::SlideStatus:
            return tr("玻片状态");
        default:
            return tr("");
        }
        //return headers.value(section);
    }
//    if (role == Qt::DisplayRole && orientation == Qt::Vertical)
//    {
//        return section + 1;
//    }

    return QVariant();
}

int UnassignedTaskListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return tasks.size();
}

int UnassignedTaskListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    //return headers.size();
    return TTaskViewColumnID::ColumnEnd;
}

bool UnassignedTaskListModel::_hasMore()
{
    bool hasMore = provider && tasks.hasMore();
    TRACE() << "hasMore : " << hasMore;
    return hasMore;
}

void UnassignedTaskListModel::_fetchOnce()
{
    TRACE();
    int count = qMin(tasks.getTotal() < 0 ? MAX_PAGE_SIZE : tasks.getTotal() - tasks.size(), MAX_PAGE_SIZE);
    IF_RETURN(count<=0);

    auto newTasks = provider->getUnassignedTaskList(rowCount(), count);
    //IF_RETURN(newTasks.size()<=0);

    int first = tasks.size();
    int last = first + std::max(0, newTasks.count()-1);
    beginInsertRows(QModelIndex(), first, last);
    if (tasks.getTotal() < 0)
    {
        tasks = newTasks;
    }
    else
    {
        tasks.append(newTasks);
    }
    endInsertRows();
}

#if 0
bool UnassignedTaskListModel::canFetchMore(const QModelIndex& /*parent*/) const
{
    bool hasMore = provider && tasks.hasMore();
    TRACE() << "hasMore : " << hasMore;
    return hasMore;
}

void UnassignedTaskListModel::fetchMore(const QModelIndex& /*parent*/)
{
    TRACE();
    int count = qMin(tasks.getTotal() < 0 ? MAX_PAGE_SIZE : tasks.getTotal() - tasks.size(), MAX_PAGE_SIZE);
    if (count <= 0)
    {
        return;
    }

    auto newTasks = provider->getUnassignedTaskList(rowCount(), count);
    int first = tasks.size();
    int last = first + std::max(0, newTasks.count()-1);
    beginInsertRows(QModelIndex(), first, last);
    if (tasks.getTotal() < 0)
    {
        tasks = newTasks;
    }
    else
    {
        tasks.append(newTasks);
    }
    endInsertRows();
    if( this->_interval >=0 )
    {
        if (  canFetchMore(QModelIndex())){
            TRACE() << "Start Timer...";
            _fetchTimer.start(this->_interval);
            _fetchTimer.setSingleShot(true);
        }
        else {
            TRACE() << "End Timer";
            _fetchTimer.stop();
        }
    }
}
#endif

QVariant UnassignedTaskListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        auto task = tasks.value(index.row());
        switch (index.column())
        {
        case TTaskViewColumnID::UID:
            return task.slideId();
        case TTaskViewColumnID::SampleId:
            return task.slideNo();
        case TTaskViewColumnID::AssayType:
            return task.assayTypeName();
        case TTaskViewColumnID::CaseNumber:
            return task.caseNo();
        case TTaskViewColumnID::CellCount:
            return task.cellCount()==-1 ? tr("") : QString::number(task.cellCount());
        case TTaskViewColumnID::ViewCount:
            return task.imageCount();
        case TTaskViewColumnID::CreateTime:
            return task.createTime();
        case TTaskViewColumnID::AssayTypeId:
            return  task.assayTypeId();
        case TTaskViewColumnID::MyStatus:
            return task.taskStatus();
        case TTaskViewColumnID::SlideStatus:
            return task.slideStatus();
        case TTaskViewColumnID::DeviceType:
            return task.deviceTypeName();
        case TTaskViewColumnID::DeviceTypeId:
            return task.deviceTypeId();
        default:
            return tr("未实现字段");
        }
    }
    return QVariant();
}
