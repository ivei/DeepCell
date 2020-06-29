#ifndef CELLITEM_H
#define CELLITEM_H
#include <QMetaType>
#include <QMap>
#include <QString>
#include <QPair>
#include <QPolygon>
#include <QJsonObject>
#include <QPixmap>

#include "categoryitem.h"
#include "dbgutility.h"


class CellPrivate
{
public:
    int         _id;
    int         _level;          //!
    QPolygon    _contours;
    QString     _contours_id;
    QString     _create_time;   //!
    QString     _user_name;
    int         _user_id;
    int         _action;     //! 1:新增,2: , 3:逻辑删除 4: 继承(提交玻片什么也不操作)
    int         _category_id;       //! 细胞类型编号, 对应于CatelogItem.id
    QString     _path;           //! 新增字段, 保存细胞图的路径.
    QRect       _rect;           //! 细胞的区域位置, 来自outlines字段.
    // 生成数据
    int         _oldType;        //! 修改前的细胞种类
    QPixmap     _pixmap;
    bool        _deleted;        //! 生成数据
};



//! 利用命令/api/actions/list查询到的细胞/核型的一个Item.
//! 一个细胞的下面会包含多个用户处理信息, 最终被放到一个列表 _values里面.
//! 并且, 认为object_id, image_id和task_id是相同的, 并且从第一个里面获取.
class CellItem
{
public:
    bool fromJson(const QJsonObject& obj);
    bool fromJsonByName(const QJsonObject& obj, const QString& name, int level);
    //! 设置文件名. 此函数以后会被废弃
    //! 返回细胞图像文件路径.
    QString url() const {return _url; }
    //! 新接口, 在细胞的path字段中直接保存着完整的细胞图像路径
    inline void setUrl(const QString& url) { _url = url; }
    //! 老接口, 没有path字段时, 需要利用contours_id字段来构造细胞路径.
    inline void setUrl(const QString& base_path, const QString& sufix){
        _url = QString("%1/objects/%2.%3").arg(base_path).arg(_values.last()._contours_id).arg(sufix);
    }
    //! 返回细胞图片的路径. 用于替代以前的setUrl, url()和makeFilePath()三个函数.
    QString fullPath() const { return _values.last()._path; }

    //! 返回最后一个值. 假定是按照时间顺序排列的.
    const CellPrivate& value() const;
    CellPrivate& value();
    inline void setValue(const CellPrivate& data) { _values.push_back(data); }
    inline bool isEmpyt() const { return this->_values.isEmpty(); }
    //! 返回/设置细胞的id
    inline int uid() const {
        return  _values.last()._id;
    }
    inline void setUid(int id) { _values.last()._id = id; }
    inline int objectId() const { return this->_object_id; }
    inline void setObjectId(int id)  { this->_object_id = id; }
    //! 返回/设置细胞对应的Image ID
    inline int imageId() const { return _image_id; }
    inline void setImageId(int id) { _image_id = id; }
    //! 返回/设置细胞对应的TaskId
    inline int taskId() const { return _task_id; }
    inline void setTaskId(int taskId) { _task_id = taskId; }
    //! 返回细胞所在的玻片的ID
    inline int slideID() const { return this->_slide_id; }
    inline QString slideNo() const { return this->_slide_no; }
    void setSlideNo(const QString& slide_no) { this->_slide_no = slide_no; }
    //! 细胞有效性判断?
    inline bool isValid() const { return objectId() > 0; }
    inline bool deleted() const { return _values.last()._deleted; }
    inline int  level() const { return _values.last()._level; }
    //! 返回细胞轮廓
    inline QPolygon countour() const { return _values.last()._contours; }
    QString contourId() const { return _values.last()._contours_id; }
    //! 返回细胞尺寸
    inline QRect rect() const { return _values.last()._rect; }
    //! 返回细胞图片
    inline const QPixmap& pixmap() const { return _values.last()._pixmap; }
    //! 返回细胞类型ID
    inline int type() const { return _values.last()._category_id; }
    //! 返回细胞类型代号
    const QString typeNo() const;
    //! 返回细胞名称(中文)
    const QString typeName() const;
    //! 设置细胞的类型信息
    void setCategoryId(int category_id);
    int getCategoryId() const;

    void setCatalog(const CategoryFamily& catalog) { this->_catalog = catalog; }
    const CategoryFamily& getCatalog() const { return this->_catalog; }

    const QString title() const {
        if( this->isValid())
            return QString("%1-%2").arg(this->typeNo()).arg(this->objectId());
        else {
            return QString("%1-%2(NEW)").arg(this->typeNo()).arg(-this->objectId());
        }
    }
    QString toString() const;
    QString showInfo() const;

    void loadContours(const QByteArray& data);
protected:
    QPolygon decodeCountours(const QJsonArray&);
    QRect decodeOutline(int obj_id, const QString& outlines);
private:
    // 公共的数据, 从数据库中读出来的数据
    int _object_id;        //! 对应于object_id.
    QString _object_no;     //!
    int _image_id;
    QString _image_no;
    QString _image_list;
    int _slide_id;
    QString _slide_no;      //! 玻片
    int _task_id;
    QString _user;
    QList<CellPrivate> _values;
    //! 生成字段信息
    QString _family;         //! 细胞的类别. 比如, 巨核细胞, 白细胞等
    QString _url;       //! 存放
    CategoryFamily  _catalog;   // 细胞本身保存的分类字典.
};


Q_DECLARE_METATYPE(CellItem)
#endif // CELLITEM_H
