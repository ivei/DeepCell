#include "cellgroupbox.h"
#include "ui_cellgroupbox.h"
#include "celllisttypefiltermodel.h"
#include <QMenu>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QDebug>
#include "../srv/dbgutility.h"
#include "../srv/commdefine.h"
#include "celllistmodel.h"

#pragma execution_character_set("utf-8")

CellGroupBox::CellGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::CellGroupBox),
    filterModel(new CellListTypeFilterModel(this))
{
    ui->setupUi(this);
    ui->listView->setModel(filterModel);

    connect(ui->pushButton, &QPushButton::clicked, this, [this](bool checked){
        ui->listView->setVisible(!checked);
        qreal ratio = filterModel->rowCount();
        ratio = ratio > 0 ? ratio / filterModel->sourceModel()->rowCount() * 100 : 0;
        ui->pushButton->setText(QString("%1[%2][%3%]").arg(title)
                                .arg(filterModel->rowCount()).arg(QString::number(ratio, 'f', 1)));
    });


    connect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &CellGroupBox::onSelectionChanged);
    //connect(this, &QGroupBox::customContextMenuRequested, this, &CellGroupBox::showContextMenu);
    //

}

CellGroupBox::~CellGroupBox()
{
    delete ui;
}

void CellGroupBox::setType(const QString &text, int type)
{
    title = text;
    typeId = type;
    filterModel->setFilter(type);
    ui->pushButton->setText(QString("%1(%2)").arg(text).arg(filterModel->rowCount()));
#if 1
    int id = type % PALATTE::ColorPalette.size();
    QString color = PALATTE::ColorPalette[id].firstColor;
    QString styleSheet = QString("QLabel{background-color: %1}").arg(color);
    TRACE() << styleSheet;
    ui->label->setStyleSheet(styleSheet);
#else
    switch (type)
    {
    case 1:
        ui->label->setStyleSheet("QLabel{background-color: red}");
        break;
    case 2:
        ui->label->setStyleSheet("QLabel{background-color: green}");
        break;
    case 3:
        ui->label->setStyleSheet("QLabel{background-color: blue}");
        break;
    case 4:
        ui->label->setStyleSheet("QLabel{background-color: darkred}");
        break;
    case 5:
        ui->label->setStyleSheet("QLabel{background-color: cyan}");
        break;
    case 6:
        ui->label->setStyleSheet("QLabel{background-color: yellow}");
        break;
    case 7:
        ui->label->setStyleSheet("QLabel{background-color: magenta}");
        break;
    case 8:
        ui->label->setStyleSheet("QLabel{background-color: darkgreen}");
        break;
    case 9:
        ui->label->setStyleSheet("QLabel{background-color: rgb(85,102,0)}");
        break;
    case 10:
        ui->label->setStyleSheet("QLabel{background-color: darkblue}");
        break;
    case 11:
        ui->label->setStyleSheet("QLabel{background-color: darkgray}");
        break;
    case 12:
        ui->label->setStyleSheet("QLabel{background-color: rgb(160,82,45)}");
        break;
    case 13:
        ui->label->setStyleSheet("QLabel{background-color: darkmagenta}");
        break;
    case 14:
        ui->label->setStyleSheet("QLabel{background-color: darkcyan}");
        break;
    case 15:
        ui->label->setStyleSheet("QLabel{background-color: gray}");
        break;
    case 16:
        ui->label->setStyleSheet("QLabel{background-color: black}");
        break;
    case 17:
        ui->label->setStyleSheet("QLabel{background-color: rgb(139,69,0)}");
        break;
    default:
        ui->label->setStyleSheet("QLabel{background-color: #FF9900}");
        break;
    }

#endif
}

/**
 * @brief CellGroupBox::setSourceModel
 * @param model
 * @note
 *
 */
void CellGroupBox::setSourceModel(QAbstractListModel *model)
{
    filterModel->setSourceModel(model);

    connect(filterModel, &QAbstractItemModel::rowsInserted, this, [this](){this->refreshRatio();});
    connect(filterModel, &QAbstractItemModel::rowsRemoved, this, [this](){this->refreshRatio();});
    connect(filterModel, &QAbstractItemModel::modelReset, this, [this](){this->refreshRatio();});
    //
    connect(filterModel->sourceModel(), &QAbstractItemModel::rowsInserted, this,[this](){this->refreshRatio();});
    connect(filterModel->sourceModel(), &QAbstractItemModel::rowsRemoved, this,[this](){this->refreshRatio();});
    connect(filterModel->sourceModel(), &QAbstractItemModel::modelReset, this,[this](){this->refreshRatio();});
    connect(filterModel->sourceModel(), &QAbstractItemModel::dataChanged, this, [this](){this->refreshRatio();});
}

QListView* CellGroupBox::view()
{
    return ui->listView;
}

void CellGroupBox::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    if (!selected.isEmpty())
    {
        emit selectionChanged();
    }
}

int CellGroupBox::getTypeId() const
{
    return typeId;
}

/**
 * @brief 刷新显示的比例
 */
void CellGroupBox::refreshRatio()
{
    int count =this->filterModel->rowCount();
    int total = qobject_cast<CellListModel *>(this->filterModel->sourceModel())->validCount();
    setVisible(count > 0);
    ui->listView->updateLayout();

    qreal ratio = total > 0 ? (qreal)count / total * 100 : 0;
    ui->pushButton->setText(QString("%1[%2][%3%]").arg(title)
                            .arg(filterModel->rowCount()).arg(QString::number(ratio, 'f', 1)));
    //TRACE() << QString("count=%1, total=%2, ration=%3").arg(count).arg(total).arg(ratio);
}

void CellGroupBox::collapse()
{
    ui->pushButton->setChecked(true);
    ui->pushButton->clicked(true);
}

void CellGroupBox::expand()
{
    ui->pushButton->setChecked(false);
    ui->pushButton->clicked(false);
}

#include <QDebug>
void CellGroupBox::showContextMenu(const QPoint &pos)
{
    auto index = ui->listView->indexAt(ui->listView->mapFrom(this, pos));
    if (!index.isValid())
    {
        return;
    }

    QMenu contextMenu(tr("Context menu"), this);

    connect(ui->actionDelete, &QAction::triggered, this, [](){
    });
    connect(ui->actionmodify, &QAction::triggered, this, [](){
    });

    contextMenu.addAction(ui->actionDelete);
    contextMenu.addAction(ui->actionmodify);

    contextMenu.exec(QCursor::pos());
}
