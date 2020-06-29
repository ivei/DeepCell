#include "reportdialog.h"
#include "ui_reportdialog.h"
#include "reporttemplatemodel.h"
#include <QInputDialog>
#include <QDebug>
#include <QMenu>

#pragma execution_character_set("utf-8")

ReportDialog::ReportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportDialog),
    model(new ReportTemplateModel(this))
{
    ui->setupUi(this);
    ui->templateView->setModel(model);
    ui->templateView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(ui->templateView->selectionModel(), &QItemSelectionModel::currentRowChanged, [this](const QModelIndex &current, const QModelIndex &previous){
        Q_UNUSED(previous);
        auto text = current.data(Qt::UserRole).value<QString>();
        ui->plainTextEdit->setPlainText(text);
    });
    connect(ui->templateView, &QTableView::customContextMenuRequested, this, &ReportDialog::showContextMenu);
    connect(ui->save, &QPushButton::clicked, [this](){
        bool ok;
        QString text = QInputDialog::getText(this, tr("保存为模板"),
                                             tr("名称:"), QLineEdit::Normal,
                                             "sure", &ok);
        if (ok && !text.isEmpty())
        {
            model->addTemplate(text, ui->plainTextEdit->toPlainText());
        }
    });
    connect(ui->cancel, &QPushButton::clicked, [this](){
        close();
    });

    connect(ui->save, &QPushButton::clicked, [](){
    });
}

ReportDialog::~ReportDialog()
{
    delete ui;
}

void ReportDialog::showContextMenu(const QPoint &pos)
{
    if (!ui->templateView->indexAt(pos).isValid())
    {
        return;
    }

    QMenu contextMenu(tr("Context menu"));
    QAction del(tr("删除"));
    contextMenu.addAction(&del);
    connect(&del, &QAction::triggered, [this, pos](){
        ui->templateView->clearSelection();
        auto index = ui->templateView->indexAt(pos);
        qDebug() << "called" << index;
        model->removeTemplate(index.data(Qt::DisplayRole).value<QString>());
    });

    contextMenu.exec(ui->templateView->mapToGlobal(pos));
}

