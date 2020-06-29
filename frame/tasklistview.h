#ifndef TASKLISTVIEW_H
#define TASKLISTVIEW_H

#include <QWidget>
#include "../srv/commdefine.h"
#include "taskproxymodel.h"
#include "tasklistmodel.h"
#include "unassignedtasklistmodel.h"
namespace Ui {
class TaskListView;
}

class QItemSelection;
class DeepLabel;
class ConnectionItem;
class TaskListView : public QWidget
{
    Q_OBJECT

public:

    enum{
        TaskTypeMyDoing = 0,
        TaskTypeMyDone  = 1,
        TaskTypeUnAssign= 2
    };

    explicit TaskListView(QWidget *parent = nullptr);
    ~TaskListView();
    //! 配置连接信息
    void setConnecton(const ConnectionItem&);
    //! 设置任务类型
    void selectTaskType(int taskType);
private slots:
    // 处理任务类型列表改变的处理slot
    void on_tasktypeListWidget_itemSelectionChanged();
    // ! 双击任务区Item的slot
    void on_cmbAssayType_currentIndexChanged(const QString &arg1);

    void on_taskTableView_doubleClicked(const QModelIndex &index);
    //! 当任务表格中的选择项发生改变后的slot
    void taskSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void on_btnQuery_clicked();
    void on_btnTake_clicked();
    void on_btnUntake_clicked();
    void on_btnRefresh_clicked();

private:
    //! 切换TaskTable中的
    void setTaskList(int type, bool cache=true);

    int getQueryAssayType();
    QString getQueryCaseNumber();
    int getQueryDeviceId();

    void setTaskModel(int type, bool cache=true);
    void resetTaskModel(int type);
    //! 将assayTypeCmb
    QSet<int> getAssayTypeRules();
    QSet<QString> getCaseNumberRules();


private:
    Ui::TaskListView *ui;
    DeepLabel *provider;
    TaskListModel  *_myTaskModel;
    UnassignedTaskListModel *_unAssignedTaskModel;
    TaskProxyModel *_myTaskProxyModel;
    TaskProxyModel *_unAssignedTaskProxyModel;
};

#endif // TASKLISTVIEW_H
