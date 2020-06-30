#include <QPushButton>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QDateTime>
#include <QMenu>
#include <QDebug>
#include <QStatusBar>
#include "cellcounter.h"
#include "ui_cellcounter.h"
#include "connectionitemdelegate.h"
#include "connetionmodel.h"
#include "tasklistview.h"
#include "connectionitem.h"
#include "connectiondialog.h"
#include "../srv/deepengine.h"
#include "../srv/deepservice.h"
#include "../srv/dbgutility.h"
#include "../version.h"
#pragma execution_character_set("utf-8")


const QString detailText ="<html><head/><body><p><span style=\" font-weight:600;\">软件名称</span>: 深析智能细胞形态显微镜图像扫描分析软件</p><p><span style=\" font-weight:600;\">软件型号</span>: CellCounter</p><p><span style=\" font-weight:600;\">版本信息</span>:</p><p><span style=\" font-weight:600;\">发布版本</span>: V%1.%2.%3.%4</p><p><span style=\" font-weight:600;\">软件说明</span>: 本软件为DCS-1000细胞医学图像分析系统配套专用软件</p></body></html>";

CellCounter::CellCounter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CellCounter),
    model(new ConnetionModel(this))
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/logo.ico"));
    ui->tabWidget->setTabsClosable(true);
    ui->tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
    ui->listWidget->setCurrentRow(0);
    ui->listView->setItemDelegate(new ConnectionItemDelegate(this));
    ui->listView->setModel(model);
    ui->detaiLabel->setText(detailText.arg(VER_V).arg(VER_R).arg(VER_C).arg(VER_B));
    connect(ui->listView, &QListView::doubleClicked, this, &CellCounter::openConnection);

    connect(ui->add, &QPushButton::clicked, this, &CellCounter::addConnection);
    connect(ui->edit, &QPushButton::clicked, this, &CellCounter::editConnection);
    connect(ui->remove, &QPushButton::clicked, this, &CellCounter::deleteConnection);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selection,const QItemSelection&){
         ui->edit->setDisabled(selection.isEmpty());
         ui->remove->setDisabled(selection.isEmpty());
    });

    connect(ui->listView, &QWidget::customContextMenuRequested, this, &CellCounter::contextMenuRequested);
    connect(ui->actionEdit, &QAction::triggered, this, &CellCounter::editConnection);
    connect(ui->actionRemove, &QAction::triggered, this, &CellCounter::deleteConnection);
    connect(ui->actionAdd, &QAction::triggered, this, &CellCounter::addConnection);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &CellCounter::tabWidgetClose);
    model->setItems(DeepEngine::instance().getConnections());


}

CellCounter::~CellCounter()
{
    TRACE() << QString("Exit MainWindow at %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    delete ui;
}

void CellCounter::addConnection()
{
    ConnectionDialog conn(this);
    conn.setWindowTitle(tr("新建连接"));
    if (conn.exec() ==  ConnectionDialog::Accepted && model->addItem(conn.getConnection()))
    {
        DeepEngine::instance().sync(model->getItems());
    }
}

void CellCounter::deleteConnection()
{
    auto index = ui->listView->currentIndex();
    if (index.isValid() && model->removeItem(index.row()))
    {
        DeepEngine::instance().sync(model->getItems());
    }
}

void CellCounter::editConnection()
{
    ConnectionDialog conn(this);
    conn.setWindowTitle(tr("编辑连接"));
    auto index = ui->listView->currentIndex();
    if (!index.isValid())
    {
        return;
    }
    conn.setConnection(index.data(Qt::UserRole).value<ConnectionItem>());
    if (conn.exec() ==  ConnectionDialog::Accepted && model->modifyItem(index.row(), conn.getConnection()))
    {
        DeepEngine::instance().sync(model->getItems());
    }
}

void CellCounter::openConnection(const QModelIndex& index)
{
    auto conn = index.data(Qt::UserRole).value<ConnectionItem>();
    TRACE() << QString("Connecting database, username: %1, host: %2:%3").arg(conn.username).arg(conn.domainName).arg(conn.port);
    if (conn.password.isEmpty())
    {
        QMessageBox::warning(this, tr("连接错误"), tr("密码为空"));
        return;
    }
    DeepService::makeUserDirs(conn.username);
    auto task = new TaskListView(this);
    try
    {
        task->setConnecton(conn);
        // 设置显示的任务类型为0(未完成任务)
        task->selectTaskType(TaskListView::TaskTypeMyDoing);
    }
    catch (const QString& errorString)
    {
        TRACE() << "connect failed: " << errorString;
        QMessageBox::warning(this, tr("连接错误"), errorString);
        return;
    }

    ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(task, conn.profileName));
}

void CellCounter::contextMenuRequested(const QPoint& pos)
{
   auto index = ui->listView->indexAt(pos);
   QMenu menu;
   if (index.isValid())
   {
       menu.addAction(ui->actionEdit);
       menu.addAction(ui->actionRemove);
   }
   else
   {
       menu.addAction(ui->actionAdd);
   }
   menu.exec(ui->listView->mapToGlobal(pos));
}

void CellCounter::tabWidgetClose(int index)
{
    QWidget* widget = ui->tabWidget->widget(index);
    ui->tabWidget->removeTab(index);
    widget->deleteLater();
}

void CellCounter::on_listWidget_itemSelectionChanged()
{
    ui->stackedWidget->setCurrentIndex(ui->listWidget->currentRow());
}
