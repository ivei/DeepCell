#ifndef CATALOGVIEW_H
#define CATALOGVIEW_H
#include <QWidget>
#include <QActionGroup>


namespace Ui {
class CatalogView;
}

class CellListModel;
class CellItem;
class CategoryItem;
class QListView;

class CatalogView : public QWidget
{
    Q_OBJECT

public:
    explicit CatalogView(QWidget *parent = nullptr);
    virtual ~CatalogView() override;
    // 此函数待删除
    //void setContent(const QMap<int, QString>& types, const QList<CellItem>& cells);
    void setContent(const QMap<int, CategoryItem>& types, const QList<CellItem>& cells);
    inline CellListModel *sourceModel() { return model; }

Q_SIGNALS:
    void cellAdded(const CellItem&);
    void cellDeleted(const CellItem&);
    void cellTypeChanged(const CellItem&, int);
    void cellSelected(const CellItem&);
public slots:
    void showAll();
    void hideAll();
    //! 处理从CellShowPage中发过来的删除细胞的信号. 在CatalogView中做完
    //! 特殊处理后,走完全相同的后续流程
    void onSigCellDelete(int cellId);
    void onSigCellTypeChanged(int cellId, int newtype, int oldtype);
    void onSigSelectCell(int cellId);
protected slots:
    void selectionChanged();
    void showContextMenu(const QPoint &pos);

private:
    Ui::CatalogView *ui;
    CellListModel *model;
    QListView* view;
    //QActionGroup* types;
};

#endif // CATALOGVIEW_H
