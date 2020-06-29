#include <QString>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include "../srv/deepservice.h"
#include "../srv/dbgutility.h"
#include "reportdlg.h"
#include "ui_reportdlg.h"

#pragma execution_character_set("utf-8")


ReportDlg::ReportDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReportDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

}



ReportDlg::~ReportDlg()
{
    delete ui;
}

void ReportDlg::setData(DeepLabel *provider, const SlideItem &taskinfo)
{
    this->provider = provider;
    this->taskInfo = taskinfo;
}

void ReportDlg::on_okBtn_clicked()
{
    QString conclusion = this->ui->resultEdit->toPlainText();
    // 因为PDF组件是将文本作为HTML来处理的, 所以将回车改为<br>
    conclusion.replace(QString("\n"), QString("<br>"));
    //QString details = this->ui->detailEdit->toPlainText();
    TRACE() << this->taskInfo.slideId() << this->taskInfo.slideNo();
    //DeepService::makeDirs(provider->getUserName());
    bool ret = provider->getReport(taskInfo.slideId(), taskInfo.slideNo(), conclusion );
    if(ret)
        QDialog::accept();
    else {
        QMessageBox::warning(this, tr("错误"), tr("生成报告失败"));
        QDialog::reject();
    }
}


void ReportDlg::on_cancelBtn_clicked()
{
    reject();
}
