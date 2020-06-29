#ifndef CELLCOUNTER_H
#define CELLCOUNTER_H

#include <QMainWindow>

namespace Ui {
class CellCounter;
}

class ConnetionModel;

class CellCounter : public QMainWindow
{
    Q_OBJECT

public:
    explicit CellCounter(QWidget *parent = nullptr);
    ~CellCounter();

protected slots:
    void addConnection();
    void deleteConnection();
    void editConnection();
    void openConnection(const QModelIndex&);
    void contextMenuRequested(const QPoint& pos);
    void tabWidgetClose(int index);
    void on_listWidget_itemSelectionChanged();

private:
    Ui::CellCounter *ui;
    ConnetionModel* model;
};

CellCounter *getCellCounterWnd();

#endif // CELLCOUNTER_H
