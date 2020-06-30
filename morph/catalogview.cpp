#include <QListView>
#include <QVBoxLayout>
#include <QDebug>
#include <QProgressDialog>
#include <QtConcurrent>
#include <QMenu>
#include "ui_catalogview.h"

#include "catalogview.h"
#include "cellgroupbox.h"
#include "celllistmodel.h"
#include "celllistview.h"
#include "../srv/cellitem.h"
#include "../srv/deeplabel.h"
#include "../srv/category.h"
#include "../srv/dbgutility.h"

#pragma execution_character_set("utf-8")

CatalogView::CatalogView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CatalogView),
    model(new CellListModel(this)),
    view(nullptr)
    //, types(new QActionGroup(this))
{
    ui->setupUi(this);

    auto layout = new QVBoxLayout(ui->scrollArea);
    layout->setContentsMargins(0, 0, 0, 0);
    ui->scrollAreaWidgetContents->setLayout(layout);
}

CatalogView::~CatalogView()
{
    delete ui;
    TRACE();
}

/**
 * @brief 填充分类视图
 * @param typeMap   细胞分类定义
 * @param cells     细胞列表
 */
void CatalogView::setContent(const QMap<int, CategoryItem>& typeMap, const QList<CellItem>& cells)
{
    auto layout = qobject_cast<QVBoxLayout*>(ui->scrollAreaWidgetContents->layout());

    for (auto iter = typeMap.begin(); iter != typeMap.end(); ++iter)
    {
//        if (iter.key() > 23)
//        {
//            break;
//        }
        auto group = new CellGroupBox(this);
        group->setSourceModel(model);
#if 0
        group->setType(iter->cnName, iter.key());
#else
        group->setType(QString("%1(%2)").arg(iter->cnName).arg(iter->typeNo), iter.key());
#endif
        group->setVisible(false);
        layout->addWidget(group);
        connect(group->view(), &QListView::doubleClicked, this, [this](const QModelIndex&index){
           emit cellSelected(index.data(Qt::UserRole).value<CellItem>());
        });
        connect(group->view(), &QWidget::customContextMenuRequested, this, &CatalogView::showContextMenu);
        connect(group, &CellGroupBox::selectionChanged, this, &CatalogView::selectionChanged);
    }

    layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));
    model->setSource(cells);
}
void CatalogView::showAll()
{
    for (auto group : findChildren<CellGroupBox*>())
    {
        group->expand();
    }
}

void CatalogView::hideAll()
{
    for (auto group : findChildren<CellGroupBox*>())
    {
        group->collapse();
    }
}

/**
 * @brief 处理CellShowPage发过来的删除细胞的信号sigCellDelete
 * @param cellId
 */
void CatalogView::onSigCellDelete(int cellId)
{
    TRACE() << QString("cellId=%1").arg(cellId);
    // 在模型中根据cellId查找CellItem
    QList<CellItem>& cells = this->sourceModel()->getCells();
    auto iter = std::find_if(cells.begin(), cells.end(), [cellId](const CellItem& c){ return cellId==c.uid();});
    if( iter == cells.end()){
        TRACE() << "can not find cell id=" << cellId << "in model";
        return;
    }
    CellItem cellItem = *iter;
    // 直接清除所有group中的选择项.
    for(auto group: this->findChildren<CellGroupBox *>())
    {
        // 先清除每个group的选择项
        group->view()->clearSelection();
    }
    // 因为从那边过来的只有一个, 就不再通过模型访问了.
    emit cellDeleted(cellItem);
}

void CatalogView::onSigCellTypeChanged(int cellId, int newtype, int oldtype)
{
    TRACE() << QString("cellId=%1, newtype=%2, oldtype=%3").arg(cellId).arg(newtype).arg(oldtype);
    // 在模型中根据cellId查找CellItem
    QList<CellItem>& cells = this->sourceModel()->getCells();
    auto iter = std::find_if(cells.begin(), cells.end(), [cellId](const CellItem& c){ return cellId==c.uid();});
    if( iter == cells.end()){
        TRACE() << "can not find cell id=" << cellId << "in model";
        return;
    }
    CellItem cellItem = *iter;
    auto cellIndex = this->sourceModel()->index(std::distance(cells.begin(), iter));
    // 直接清除所有group中的选择项.
    for(auto group: this->findChildren<CellGroupBox *>())
    {
        // 先清除每个group的选择项
        group->view()->clearSelection();
    }
    emit cellTypeChanged(cellItem, newtype);
    this->model->setData(cellIndex, newtype, CellListModel::TYPE_ROLE);
    for(auto group: this->findChildren<CellGroupBox *>())
    {
        if(group->getTypeId()==newtype || group->getTypeId()==oldtype){
            qobject_cast<CellListView *>(group->view())->updateLayout();
        }
    }
}

/**
 * @brief 将id值未cellId的细胞设置为选择.
 * @param cellId
 */
void CatalogView::onSigSelectCell(int cellId)
{
    TRACE() << QString("cellId=%1").arg(cellId);
    // 在模型中查找cellId的Item
    QList<CellItem>& cells = this->sourceModel()->getCells();
    auto iter = std::find_if(cells.begin(), cells.end(), [cellId](const CellItem& c){ return cellId==c.uid();});
    if( iter == cells.end()){
        TRACE() << "can not find cell id=" << cellId << "in model";
        return;
    }
    // 获取细胞的类别

    int typeId = iter->getCategoryId();
    // 细胞在sourceModel中的index
    auto cellIndex = this->sourceModel()->index(std::distance(cells.begin(), iter));
    // 细胞在哪个Group中:
    CellGroupBox *cellGroup = nullptr;
    for(auto group: this->findChildren<CellGroupBox *>())
    {
        // 先清除每个group的选择项
        group->view()->clearSelection();
        // 检查typeId
        if( group->getTypeId() == typeId){
            cellGroup = group;
        }
    }

    // 获取要选中的cell所在的group对应的模型(proxyModel):
    auto groupModel = qobject_cast<QSortFilterProxyModel *>(cellGroup->view()->model());
    // 将全局的index转化为在proxy中的index:
    auto groupIndex = groupModel->mapFromSource(cellIndex);

    cellGroup->view()->selectionModel()->select(groupIndex,QItemSelectionModel::Select);


}

void CatalogView::selectionChanged()
{
    TRACE();
    auto box = qobject_cast<CellGroupBox *>(sender());
    if (view && view != box->view())
    {
        view->clearSelection();
    }
    view = box->view();
}

/**
 * @brief 处理上下文菜单, 实现删除细胞和修改细胞类别的功能
 * @param pos
 */
void CatalogView::showContextMenu(const QPoint &pos)
{
    auto view = qobject_cast<CellListView*>(sender());
    if (!view->indexAt(pos).isValid())
    {
        return;
    }
    //auto t = view->indexAt(pos).data(Qt::UserRole).value<CellItem>().value()
    CellItem cell_item = view->indexAt(pos).data(Qt::UserRole).value<CellItem>();
    // 获取细胞的分类字典, 用来构造Action对象
    const auto& cell_catalog = cell_item.getCatalog();

    QActionGroup *typeGroup = new QActionGroup(this);
    for(const auto& item: cell_catalog)
    {
//        // 在这里应对老数据库没有family的情况.
//        if( cell_item.getFamily()=="01" && item.id>23)
//            continue;
        auto action = new QAction(item.cnName);
        action->setCheckable(true);
        action->setData(item.id);
        typeGroup->addAction(action);
    }
    typeGroup->setExclusive(true);

    QMenu contextMenu(tr("Context menu"));
    QMenu typesMenu(tr("修改细胞类别"));

    connect(ui->actionDelete, &QAction::triggered, this, [this, view](){
        TRACE() << QString("Delete Cell MenuItem triggered");
        auto model = qobject_cast<QSortFilterProxyModel*>(view->model());
        QModelIndexList sourceIndexes;
        for (auto index : view->selectedIndexes())
        {
            sourceIndexes.append(model->mapToSource(index));
        }
        view->clearSelection();
        for (auto index : sourceIndexes)
        {
            //qDebug() << index.data(Qt::UserRole).value<CellItem>().id() << endl;
            //qDebug() << index.data(Qt::UserRole).value<CellItem>().value().id << endl;
            TRACE() << QString("notify to delete cell. id=%1").arg(index.data(Qt::UserRole).value<CellItem>().uid());
            emit cellDeleted(index.data(Qt::UserRole).value<CellItem>());
//            this->model->setData(index, true, CellListModel::STATUS_ROLE);
        }
    });

    connect(typeGroup, &QActionGroup::triggered, this, [this, view](QAction *action){
        TRACE() << QString("Change Cell Type MenuItem triggered");
        auto model = qobject_cast<QSortFilterProxyModel*>(view->model());
        QModelIndexList sourceIndexes;
        for (auto index : view->selectedIndexes())
        {
            sourceIndexes.append(model->mapToSource(index));
        }
        view->clearSelection();
        for (auto index : sourceIndexes)
        {
            int type = action->data().toInt();
            CellItem cell = index.data(Qt::UserRole).value<CellItem>();
            //qDebug() << "change" << index.data(Qt::UserRole).value<CellItem>().id();
            TRACE() << QString("notify to change cell(id=%1)'s type from %2 to %3")
                       .arg(cell.uid()).arg(type).arg(cell.type());
            emit cellTypeChanged(index.data(Qt::UserRole).value<CellItem>(), type);
            this->model->setData(index, type, CellListModel::TYPE_ROLE);
        }
        view->updateLayout();
    });

    auto t = view->indexAt(pos).data(Qt::UserRole).value<CellItem>().value()._category_id;
    contextMenu.addAction(ui->actionDelete);
    contextMenu.addMenu(&typesMenu);
    for (auto a : typeGroup->actions())
    {
        if (t == a->data())
        {
            a->setChecked(true);
        }
    }
    typesMenu.addActions(typeGroup->actions());

    contextMenu.exec(view->mapToGlobal(pos));

    qDeleteAll(typeGroup->actions());
    delete typeGroup;
}


