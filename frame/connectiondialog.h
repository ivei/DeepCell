#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectionDialog;
}

class ConnectionItem;
class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    ~ConnectionDialog();

    void setConnection(const ConnectionItem&);
    ConnectionItem getConnection();

protected slots:

protected:
    void accept() override;
private:
    Ui::ConnectionDialog *ui;
};

#endif // CONNECTIONDIALOG_H
