
#include <QMenu>
#include <QDialog>
#include <QVBoxLayout>
#include <QSplitter>
#include <QException>
#include <QMessageBox>
#include <QStatusBar>

#include "../srv/deeplabel.h"
#include "../srv/slideitem.h"
#include "../srv/commdefine.h"
#include "../srv/dbgutility.h"
#include "morph/analyzerwindow.h"

#include "connectionitem.h"
#include "tasklistview.h"
#include "ui_tasklistview.h"
#include "cellcounter.h"
#include "tasklistmodel.h"
#include "unassignedtasklistmodel.h"

#pragma execution_character_set("utf-8")

TaskListView::TaskListView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TaskListView)
    , provider(new DeepLabel(this))
    , _myTaskModel(nullptr)
    , _unAssignedTaskModel(nullptr)
    , _myTaskProxyModel(nullptr)
    , _unAssignedTaskProxyModel(nullptr)
{
    ui->setupUi(this);
#if 0   // 这几个功能没有意义. 不再提供.
    QMenu *menu = new QMenu(this);
    menu->addAction("修改头像");
    menu->addAction("修改密码");
    menu->addAction("退出");
    ui->user->setMenu(menu);
#endif

    this->ui->cmbDeviceId->hide();  // 根据设备ID过滤的使用场景看不到, 先隐藏
    ui->taskTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->taskTableView->setColumnHidden(0, true);
    //ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui->tableView->setColumnHidden(0, true);

}

TaskListView::~TaskListView()
{
    delete ui;
}

void TaskListView::setConnecton(const ConnectionItem& item)
{
    provider->setHost(item.domainName, item.port);
    provider->setCredential(item.username, item.password, "user");
    bool rc = false;
    TRACE() << "login...";
    auto errorString = provider->login(&rc);
    if (!rc)
    {
        throw errorString;
    }
    ui->user->setText(item.username);
}

/**
 * @brief 设置当前的任务类型.
 * @param taskType 显示任务类别.类别和tasktypeListWidget中的Item的序号定义.
 *                 0(TaskTypeMyDoing) 表示未完成任务
 *                 1(TaskTypeMyDone) 表示已完成任务
 *                 2(TaskTypeUnAssign) 表示可领取任务
 * @note
 *      这个函数会设置tasktypeListWidget的选择项.
 *      而tasktypeListWIDget的选择项发生变化后会触发on_tasktypeListWidget_itemSelectionChanged
 *      在后者中具体处理数据刷新等工作.
 */
void TaskListView::selectTaskType(int taskType)
{
    TRACE() << "selecting taskType is" << taskType;
    if( taskType>=TaskTypeMyDoing && taskType<=TaskTypeUnAssign)
    {
        ui->tasktypeListWidget->setCurrentRow(taskType);
    }
}


void TaskListView::on_tasktypeListWidget_itemSelectionChanged()
{
    this->setTaskList(ui->tasktypeListWidget->currentRow(), true);
}

/**
 * @brief 用户更新了样本类型选择下拉框后的处理, 会直接过滤结果. 如果case_number里面有, 则继续保持
 * @param arg1
 */
void TaskListView::on_cmbAssayType_currentIndexChanged(const QString &/*arg1*/)
{
    // 根据内容确定类型
    //auto pModel = this->getProxyModel();
    auto pModel = qobject_cast<TaskProxyModel *>(ui->taskTableView->model());
    IF_RETURN(pModel == nullptr);
    pModel->setAssayTypeRule(this->getAssayTypeRules(), true);
}

void TaskListView::on_taskTableView_doubleClicked(const QModelIndex &index)
{

    ui->taskTableView->clearSelection();
    ui->taskTableView->setCurrentIndex(index);
    if( ui->tasktypeListWidget->currentRow() <=TaskTypeMyDone)
    {
        TRACE() << QString("doubleClick in %1 (MyDoing / MyDone)").arg(ui->tasktypeListWidget->currentRow());
        // 获取数据.
        auto slide = index.data(Qt::UserRole).value<SlideItem>();
        // 检查AssayTypeID. 只处理外周血和骨髓, 其他的不处理
        if( slide.assayTypeId() == TAssayType::BM_TYPE || slide.assayTypeId()==TAssayType::PB_TYPE)
        {
            TRACE() << QString("Open slide: assayType(%1|%2), slideId: %3|%4 ")
                       .arg(slide.assayTypeId()).arg(slide.assayTypeName())
                       .arg(slide.slideId()).arg(slide.slideNo())
                       ;
            AnalyzerWindow analyzer(this);
            analyzer.initialize(provider, slide);
            analyzer.exec();
        }
        else {
            QMessageBox::warning(this, tr("提示"), tr("当前仅支持外周血和骨髓样本. "));
            return;
        }
    }
    else {
        QMessageBox::warning(this, tr("提示"), tr("不支持打开未领用的任务!"));
        return;
    }

}



void TaskListView::taskSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    //if (ui->listWidget->currentRow() == 0)
    int type = ui->tasktypeListWidget->currentRow();
    if( type == TaskTypeMyDoing){ // 在未完成任务中, 确定是否是能退领按钮
        ui->btnUntake->setDisabled(selected.empty());
    }
    else if (type==TaskTypeUnAssign) {  // 已
        ui->btnTake->setDisabled(selected.empty());
    }
    else {

    }
}

/**
 * @brief Query按钮按下, 更新TaskTable的更新. 处理方式和listWidget的方式相同.
 */
void TaskListView::on_btnQuery_clicked()
{
    TRACE();
    auto pModel = qobject_cast<TaskProxyModel *>(ui->taskTableView->model());
    IF_RETURN(pModel == nullptr);
    pModel->setCaseNumberRule(this->getCaseNumberRules(), true);
}

void TaskListView::on_btnTake_clicked()
{
    TRACE();
    auto selection = ui->taskTableView->selectionModel()->selectedRows(0);
    for(auto task : selection)
    {
        TRACE() << QString("Take task.uid: %1").arg(task.data().toInt());
        this->provider->takeTask(task.data().toInt());
    }
    // 刷新两个模型, 但是并不真正读取数据.
    this->setTaskModel(2, false);
    this->setTaskModel(0, false);
    this->selectTaskType(0);
    //this->ui->taskTableView->setModel(this->_myTaskProxyModel);

}

void TaskListView::on_btnUntake_clicked()
{
    TRACE();
    auto selection = ui->taskTableView->selectionModel()->selectedRows(0);
    for(auto task: selection)
    {
        TRACE() << "untake: " << task.data().toInt();
        provider->untakeTask(task.data().toInt());
    }
    this->setTaskModel(0, false);
    this->setTaskModel(2, false);
    this->selectTaskType(2);
}

/**
 * @brief 强制数据库刷新.
 */
void TaskListView::on_btnRefresh_clicked()
{
    int type = this->ui->tasktypeListWidget->currentRow();
    this->setTaskList(type, false);
}



/**
 * @brief 设置taskTableView中要显示的任务的类别.
 * @param type  任务的类别
 *        - 0: 未完成任务
 *        - 1: 我的已完成任务
 *        - 2: 我可以领用的任务
 * @parm cache 是否强制刷新. 默认未true, 表示不刷新.
 *          当需要重新加载数据时, 设置为false
 */
void TaskListView::setTaskList(int taskType, bool cache /*= true*/)
{
    TRACE();
    //TRACE() << QString("param type is: %1").arg(type);
    // 定义不显示的列.
    const int hide_cols[]  {
            TTaskViewColumnID::UID,
            TTaskViewColumnID::AssayTypeId,
            //TTaskViewColumnID::DeviceType,
            TTaskViewColumnID::DeviceTypeId,
            TTaskViewColumnID::MyStatus,
            TTaskViewColumnID::SlideStatus,
        };
    ui->btnTake->setVisible(taskType==TaskTypeUnAssign);   // 领取按钮只有在类型2时才显示
    ui->btnUntake->setVisible(taskType==TaskTypeMyDoing); // 退领按钮只有在类型0的时候才显示.

    this->setTaskModel(taskType, cache);

    // 这个处理其实没有太大的必要.
    connect(ui->taskTableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this,       &TaskListView::taskSelectionChanged);

    // 隐藏不需要的列
    for(auto i : hide_cols)
    {
        this->ui->taskTableView->setColumnHidden(i, true);
    }
    // 默认按照创建时间降序排序, 新的在前面.
    ui->taskTableView->setSortingEnabled(true);
    ui->taskTableView->sortByColumn(TTaskViewColumnID::CreateTime, Qt::DescendingOrder);
    // 用两种颜色.
    //ui->taskTableView->setAlternatingRowColors(true);
    //ui->taskTableView->sortByColumn(1, Qt::AscendingOrder);

}






int TaskListView::getQueryAssayType()
{
    switch (ui->cmbAssayType->currentIndex()) {
    case 0:
        return TAssayType::ALL_TYPE;
    case 1:
        return TAssayType::BM_TYPE;
    case 2:
        return TAssayType::PB_TYPE;
    default:
        return TAssayType::ALL_TYPE;
    }
}

QString TaskListView::getQueryCaseNumber()
{
    return ui->editCaseNumber->text();
}

int TaskListView::getQueryDeviceId()
{
    return 0;
}

/**
 * @brief 设置TaskView的模型.
 * @param type  task的类型.
 *      0: 未完成任务
 *      1: 已完成任务
 *      2: 可领用任务
 * @param cache 是否重用已有的模型. 若为true, 则当模型非空时, 不会重新创建.
 */
void TaskListView::setTaskModel(int type, bool cache /*= true*/)
{
    TRACE();

    if( type==0 || type==1)
    {
        TaskListModel *myTaskModel;
        TaskProxyModel *myTaskProxyModel;
        // 当模型指针为空, 或者模型指针不为空,但是要求强制刷新时, 需要重新建立模型并设置.
        if( (this->_myTaskModel==nullptr) || (this->_myTaskModel!=nullptr && cache==false))
        {
            myTaskModel = new TaskListModel(this, FetchInterval);
            myTaskProxyModel = new TaskProxyModel(this);
            if( this->_myTaskModel!=nullptr && cache==false)
            {
                myTaskProxyModel->copyFilters(this->_myTaskProxyModel);
                this->_myTaskProxyModel->deleteLater();
                this->_myTaskModel->deleteLater();
            }
            this->_myTaskModel = myTaskModel;
            this->_myTaskProxyModel = myTaskProxyModel;
            this->_myTaskModel->setProvider(this->provider);
            this->_myTaskModel->startAutoFetch();
            this->_myTaskProxyModel->setSourceModel(this->_myTaskModel);
        }

        QSet<int> my_status_rule;
        switch (type) {
        case 0:
            my_status_rule.insert(TSlideStatus::SLIDE_DOING);
            break;
        case 1:
            my_status_rule.insert(TSlideStatus::SLIDE_DONE);
            break;
        default:
            my_status_rule.clear();
            break;
        }
        this->_myTaskProxyModel->setMyStatusRule(my_status_rule, true);
        ui->taskTableView->setModel(this->_myTaskProxyModel);
    }
    else if(type==2)
    {
        UnassignedTaskListModel *unAssignedTaskModel;
        TaskProxyModel *unAssignedProxyModel;
        bool needCreate = (this->_unAssignedTaskModel==nullptr) || (this->_unAssignedTaskModel!=nullptr && cache==false);
        bool needCopy   = this->_unAssignedTaskModel!=nullptr && cache==false;
        if( needCreate){
            unAssignedTaskModel = new UnassignedTaskListModel(this, FetchInterval);
            unAssignedProxyModel = new TaskProxyModel(this);
        }
        if( needCopy){
            unAssignedProxyModel->copyFilters(this->_unAssignedTaskProxyModel);
            this->_unAssignedTaskProxyModel->deleteLater();
            this->_unAssignedTaskModel->deleteLater();
        }
        if( needCreate){
            this->_unAssignedTaskModel = unAssignedTaskModel;
            this->_unAssignedTaskProxyModel = unAssignedProxyModel;
            this->_unAssignedTaskModel->setProvider(this->provider);
            this->_unAssignedTaskModel->startAutoFetch();
            this->_unAssignedTaskProxyModel->setSourceModel(this->_unAssignedTaskModel);
        }
        ui->taskTableView->setModel(this->_unAssignedTaskProxyModel);
    }
    else {
        ERROR() << QString("invalid type: %1").arg(type);
    }
}



QSet<int> TaskListView::getAssayTypeRules()
{
    QString text = ui->cmbAssayType->currentText();
    if( text == tr("所有类型")){
        return {};
    }
    else if (text == tr("骨髓")) {
        return {1};
    }
    else if (text == tr("外周血")) {
        return {2};
    }
    else if (text == tr("核型")){
        return {4};
    }
    else if (text == tr("形态")) {
        return {1,2};
    }
    return {};
}

QSet<QString> TaskListView::getCaseNumberRules()
{
    QString text = ui->editCaseNumber->text();
    if( text.length()==0){
        return {};
    }
    else {
        return {text};
    }
}





