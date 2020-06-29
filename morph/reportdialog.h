#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>

class ReportTemplateModel;
namespace Ui {
class ReportDialog;
}

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportDialog(QWidget *parent = nullptr);
    ~ReportDialog();

public slots:
    void showContextMenu(const QPoint &pos);
private:
    Ui::ReportDialog *ui;
    ReportTemplateModel *model;
};

#endif // REPORTDIALOG_H
