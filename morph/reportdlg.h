#ifndef REPORTDLG_H
#define REPORTDLG_H

#include <QDialog>

#include "../srv/deeplabel.h"
#include "../srv/slideitem.h"

namespace Ui {
class ReportDlg;
}
class DeepLabel;
class SlideItem;

class ReportDlg : public QDialog
{
    Q_OBJECT

public:
    ReportDlg(QWidget *parent = nullptr );
    ~ReportDlg();
    void setData(DeepLabel* provider, const SlideItem& taskinfo);

private slots:
    void on_okBtn_clicked();



    void on_cancelBtn_clicked();

private:
    Ui::ReportDlg *ui;
    DeepLabel *provider;
    SlideItem taskInfo;
};

#endif // REPORTDLG_H
