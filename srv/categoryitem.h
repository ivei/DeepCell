#ifndef CELLTYPEITEM_H
#define CELLTYPEITEM_H

#include <QMetaType>
#include <QString>
#include <QMap>
class CategoryItem
{
public:
    int         id;     //! 类别ID
    QString     cnName; //! 类别名称(中文)
    QString     enName; //! 类别名称(英文)
    QString     typeNo; //! 类别代号(就是英文)
    QString     family; //!

    void fromJson(const QJsonObject& obj);
    QString getName(int language=0) const;

    QString toString() {return QString("id=%1, cnName=%2, enName=%3, no=%4, family=%5").arg(id).arg(cnName).arg(enName).arg(typeNo).arg(family);}
};
Q_DECLARE_METATYPE(CategoryItem)

typedef QMap<int, CategoryItem> CategoryFamily;
typedef QMap<QString, CategoryFamily> Catalog;



//! 从一个总的集合中得到指定family的分类子集定义. 注意: 白细胞 和巨核细胞被合并处理.
CategoryFamily filtFamily(const CategoryFamily& catalog, const QString& familyId);

#endif // CELLTYPEITEM_H
