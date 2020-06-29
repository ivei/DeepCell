#include <QDebug>

#include "../srv/dbgutility.h"
#include "../srv/deeplabel.h"
#include "../srv/slideitem.h"

#include "tasklistmodel.h"

#pragma execution_character_set("utf-8")

//static QStringList headers = {/*"UID",*/ "样本编号", "样本类型",  "病历号", "视野个数","扫描时间"/*, "Status"*/};

TaskListModel::TaskListModel(QObject *parent, int interval /*= -1*/)
    //: QAbstractTableModel(parent)
    : BatchModel (interval,parent)
    , provider(nullptr)
    , _interval(interval)
{
    connect(&this->_fetchTimer, &QTimer::timeout, [this](){this->fetchMore(QModelIndex());});
}

TaskListModel::~TaskListModel()
{
    TRACE() << "TaskListModel destroyed" << endl;
}

QVariant TaskListModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            return QVariant();
        }
        return QVariant();
        //return headers.value(section);
    }
//    if (role == Qt::DisplayRole && orientation == Qt::Vertical)
//    {
//        return section + 1;
//    }

    return QVariant();
}

int TaskListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return tasks.size();
}

int TaskListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    //return headers.size();
    return TTaskViewColumnID::ColumnEnd;
}

QVariant TaskListModel::data(const QModelIndex &index, int role) const
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
            return  task.taskStatus();
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
    else if (role == Qt::UserRole)
    {
        return QVariant::fromValue(tasks.value(index.row()));
    }

    return QVariant();
}

#if 0
void TaskListModel::fetchAll()
{
#if 0
    while (canFetchMore(QModelIndex())) {
        fetchMore(QModelIndex());
    }
#else
    while (this->_hasMore()) {
        this->_fetchOnce();
    }
#endif
}

void TaskListModel::startAutoFetch()
{
    this->_fetchTimer.setSingleShot(true);
    this->_fetchTimer.callOnTimeout([&](){
        this->_fetchOnce();
        if( !this->_hasMore()){
            this->_fetchTimer.stop();
        }
        else {
            this->_fetchTimer.start(this->_interval);
        }
    });
    this->_fetchTimer.start(0);
}
#endif

bool TaskListModel::_hasMore()
{
    bool hasMore = provider && tasks.hasMore();
    TRACE() << "_hasMore : " << hasMore;

    return hasMore;
}

void TaskListModel::_fetchOnce()
{
    TRACE();
    // 计算一次要下载的数量
    int count = qMin( tasks.getTotal()<0 ? MAX_PAGE_SIZE : tasks.getTotal()-tasks.size()
                    , MAX_PAGE_SIZE);
    IF_RETURN(count<=0);
    // 获取数据. 如果获取不到, 则直接返回. 避免后面beginInsertRows()出错.
    auto newTasks = provider->getTaskList(rowCount(), /*count*/MAX_PAGE_SIZE);


    int first = tasks.size();
    int last  = first + std::max(0, newTasks.count()-1);    // 有了前面的保护, 这里的已经是不需要了.
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

bool TaskListModel::canFetchMore(const QModelIndex &/*parent*/) const
{
#if 0
    bool hasMore = provider && tasks.hasMore();
    TRACE() << "hasMore : " << hasMore;

    return hasMore;
#else
    return false;
#endif
}

void TaskListModel::fetchMore(const QModelIndex &/*parent*/)
{
#if 0
    TRACE();
    int count = qMin(tasks.getTotal() < 0 ? MAX_PAGE_SIZE : tasks.getTotal() - tasks.size(), MAX_PAGE_SIZE);
    if (count <= 0)
    {
        return;
    }
    // 从数据库中读取最多count条记录
    auto newTasks = provider->getTaskList(rowCount(), count);
    int first = tasks.size();
    // 要保证last必须大于等于first!
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
    if( this->_interval>=0)
    {
        if( canFetchMore(QModelIndex())){
            TRACE() << "Start Timer for next frame...";
            this->_fetchTimer.setSingleShot(true);
            this->_fetchTimer.start(this->_interval);
        }
        else {
            TRACE() << "Stop Timer...";
            this->_fetchTimer.stop();
        }
    }
#endif
}


