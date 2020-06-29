#ifndef CELLLISTVIEW_H
#define CELLLISTVIEW_H
#include <QListView>

class CellListView : public QListView
{
    Q_OBJECT
public:
    using QListView::QListView;
    using QListView::selectedIndexes;

public slots:
    void updateLayout();
protected:
    void resizeEvent(QResizeEvent *e) override;
};

#endif // CELLLISTVIEW_H
