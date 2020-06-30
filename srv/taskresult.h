#ifndef TASKRESULT_H
#define TASKRESULT_H
#include <QList>
#include <QMap>
#include "cellitem.h"
#include "imageitem.h"
#include "category.h"



class TaskResult
{
public:
    TaskResult(int id);
    TaskResult(int id, int type);
    inline int id() const { return _id; }
    inline int type() const { return _type; }
    inline void setType(int type) { _type = type; }
    inline void append(const CellItem& cell) { _cells.append(cell); }
    inline void append(const QList<ImageItem>& images ) { _images.append(images); }
    inline void append(const ImageItem& cell) { _images.append(cell); }
    inline void append(const QList<CellItem> cells) { _cells.append(cells); }
    inline const QList<CellItem>& cellList() const { return _cells; }
    inline const QList<ImageItem>& imageList() const { return _images; }
    QList<ImageItem>& images() { return _images; }
    QList<CellItem>& cells() {return _cells; }

    const QMap<int, CategoryItem>& catalogs() const { return _catalogs; }
    void catalogs(const QMap<int, CategoryItem>& d) {_catalogs = d; }

private:
    int _id;
    int _type;
    QList<CellItem>     _cells;      //! 细胞列表
    QList<ImageItem> _images;        //! 视野图
    QMap<int, CategoryItem> _catalogs;   //! 分类字典
};

#endif // TASKRESULT_H
