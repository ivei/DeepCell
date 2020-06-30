#include "celllistview.h"
#include <QDebug>
#include <QLayout>
#include <QResizeEvent>
#include <QtMath>

#pragma execution_character_set("utf-8")

void CellListView::updateLayout()
{
    int width = this->width();
    int count = model() ? model()->rowCount() : 0;
    int column = (width - spacing()) / (sizeHintForColumn(0) + spacing());
    int row = qCeil(qreal(count) / qMax(1, column));
    int height = row * (spacing() + sizeHintForRow(0)) + spacing();
    setFixedHeight(height);
    update();
}

void CellListView::resizeEvent(QResizeEvent *e)
{
    auto width = e->size().width();
    if (e->oldSize().width() != width)
    {
        int count = model() ? model()->rowCount() : 0;
        int column = (width - spacing()) / (sizeHintForColumn(0) + spacing());
        int row = qCeil(qreal(count) / qMax(1, column));
        int height = row * (spacing() + sizeHintForRow(0)) + spacing();
        setFixedHeight(height);
    }
    QListView::resizeEvent(e);
}
