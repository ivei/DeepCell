#include "aboutdialog.h"
#include "ui_aboutdialog.h"

const QString titleTextTemplate = QObject::tr("关于CELL COUNTER");
const QString detailTextTemplate = QObject::tr("<html><head/><body><p><span style=\" font-weight:600;\">软件名称</span>: 深析智能细胞形态显微镜图像扫描分析软件</p><p><span style=\" font-weight:600;\">软件型号</span>: CellCounter</p><p><span style=\" font-weight:600;\">版本信息</span>:</p><p><span style=\" font-weight:600;\">发布版本</span>: V%1.%2.%3.%4</p><p><span style=\" font-weight:600;\">软件说明</span>: 本软件为DCS-1000细胞医学图像分析系统配套专用软件</p></body></html>");
const QString copyTextTemplate = QObject::tr("<html><head/><body><p>Copyright 2018-2020 Deepcyto. All Rights Reserved.</p><p>深圳深析智能有限公司版权所有</p><p>www.deepcyto.cn</p><p>了解更多信息, 请参阅我们的主页</p></body></html>");
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->detailText->setText(detailTextTemplate.arg(1).arg(1).arg("03").arg("20200620"));

}

AboutDialog::~AboutDialog()
{
    delete ui;
}
