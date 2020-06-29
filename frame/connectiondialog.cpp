#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include "connectionitem.h"
#include <QRegExpValidator>
#include <QIntValidator>

#pragma execution_character_set("utf-8")

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);
    //connect(ui->test, &QPushButton::clicked, this, &ConnectionDialog::runTest);
    connect(ui->cancel, &QPushButton::clicked, this, [this](){
        emit this->reject();
    });
    connect(ui->confirm, &QPushButton::clicked, this, [this](){
        emit this->accept();
    });
    ui->profileName->setValidator(new QRegExpValidator(QRegExp("^[^\\s].*$"), this));
    ui->domainName->setValidator(new QRegExpValidator(QRegExp("^[a-z0-9]+(?:[\\-\\.]{1}[a-z0-9]+)*\\.[a-z]{2,7}$"), this));
    ui->port->setValidator(new QIntValidator(1, 65536, this));
    ui->username->setValidator(new QRegExpValidator(QRegExp("^[^\\s].*$"), this));
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::setConnection(const ConnectionItem& item)
{
    ui->profileName->setText(item.profileName);
    ui->domainName->setText(item.domainName);
    ui->port->setText(item.port == -1 ? "" : QString::number(item.port));
    ui->username->setText(item.username);
    ui->password->setText(item.password);
    ui->remember->setChecked(ui->remember);
}

ConnectionItem ConnectionDialog::getConnection()
{
    ConnectionItem conn;
    conn.profileName = ui->profileName->text().trimmed();
    conn.domainName = ui->domainName->text().trimmed();
    conn.port = ui->port->text().isEmpty() ? -1 : ui->port->text().toInt();
    conn.username = ui->username->text().trimmed();
    conn.password = ui->password->text();
    conn.remember = ui->remember->isChecked();
    return conn;
}



void ConnectionDialog::accept()
{
    for (auto editor: {ui->profileName, ui->domainName, ui->username})
    {
        if (editor->text().isEmpty())
        {
            editor->setFocus();
            return;
        }
    }
    QDialog::accept();
}
