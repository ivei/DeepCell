#ifndef SIMPLEPROGRESSDLG_H
#define SIMPLEPROGRESSDLG_H

#include <QProgressDialog>

class SimpleProgressDlg : public QProgressDialog
{
public:
    SimpleProgressDlg(QWidget *parent = nullptr);
};

#endif // SIMPLEPROGRESSDLG_H
