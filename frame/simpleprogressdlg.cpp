#include "simpleprogressdlg.h"
#pragma execution_character_set("utf-8")



SimpleProgressDlg::SimpleProgressDlg(QWidget *parent)
    : QProgressDialog (parent, Qt::FramelessWindowHint)
{
    this->setRange(0,0);
    this->setMinimumWidth(600);
    this->setCancelButton(nullptr);
    this->setLabel(nullptr);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}
