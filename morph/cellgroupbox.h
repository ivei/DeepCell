#ifndef CELLGROUPBOX_H
#define CELLGROUPBOX_H

#include <QGroupBox>
#include <QAbstractListModel>

namespace Ui {
class CellGroupBox;
}

class CellListTypeFilterModel;
class QListView;
class QItemSelection;
class CellGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit CellGroupBox(QWidget *parent = nullptr);
    ~CellGroupBox();
    void setType(const QString &text, int type);
    void setSourceModel(QAbstractListModel *model);
    QListView* view();
    int getTypeId() const;
    void refreshRatio();

signals:
    void selectionChanged();
public slots:
    void collapse();
    void expand();

protected slots:
    void showContextMenu(const QPoint &pos);
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
private:
    Ui::CellGroupBox *ui;
    CellListTypeFilterModel *filterModel;
    QString title;  //! 类型名称
    int typeId;       //! 类型ID
};

#endif // CELLGROUPBOX_H
