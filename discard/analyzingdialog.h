#ifndef ANALYZINGDIALOG_H
#define ANALYZINGDIALOG_H

#include <QDialog>

namespace Ui {
class AnalyzingDialog;
}

class AnalyzingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnalyzingDialog(QWidget *parent = nullptr);
    ~AnalyzingDialog();

private:
    Ui::AnalyzingDialog *ui;
};

#endif // ANALYZINGDIALOG_H
