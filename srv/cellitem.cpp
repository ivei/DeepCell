#include "cellitem.h"
#include <QVariantMap>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDebug>
#include "categoryitem.h"
#include "dbgutility.h"
#pragma execution_character_set("utf-8")


bool CellItem::fromJson(const QJsonObject& obj)
{
    // 每个obj里面可能会包含多个数据段,每个对应于一个user的.
    for (auto value : obj)  // value对应于一个user名下的
    {
        if (!value.isObject())
        {
            continue;
        }
        auto item = value.toObject();

        _object_id  = item.value("object_id").toInt();
        _image_id   = item.value("image_id").toInt();
        _task_id    = item.value("task_id").toInt();
        _slide_id   =item.value("slide_id").toInt();

        CellPrivate _celldata;
        _celldata._id        = item.value("id").toInt();
        _celldata._oldType   = item.value("object_type_id").toInt();
        _celldata._category_id      = _celldata._oldType;
        _celldata._level     = item.value("level").toInt();
        _celldata._contours  = decodeCountours(item.value("contours").toArray());
        _celldata._contours_id  = item.value("contours_id").toString();
        _celldata._user_name = item.value("username").toString();
        _celldata._user_id   = item.value("user_id").toInt();
        _celldata._pixmap    = QPixmap();
        _celldata._action    = item.value("action").toInt();
        _celldata._deleted   = _celldata._action==3;
        // 注意, path字段是全路径名, 新增的字段, 可能不存在. 当不存在时为空.
        _celldata._path      = item.value("path").toString();
        _celldata._rect      = decodeOutline(_object_id, item.value("outlines").toString());

        _values.push_back(_celldata);

   }

    return false;
}

/**
 * @brief 读取当前用户的细胞数据.
 * @param obj:  json数据
 * @param name: 当前用户的level
 * @param level: 当前用户的level
 * @return
 * @note
 *      本函数暂不考虑数据不存在的情况
 *      本函数会清除已有的所有数据.
 * @note
 *      QJsonObject的结构:
 *      - username1
 *      - username2
 *
 */
bool CellItem::fromJsonByName(const QJsonObject &obj, const QString &name, int level)
{
    Q_UNUSED(level)
    _values.clear();
    // 直接按照用户名查找. 如果找不到, 当前直接返回失败.
    auto item = obj.value(name).toObject();
    if( item.isEmpty()){
        ERROR() << QString("Can not find user %1's data").arg(name);
        return false;
    }
    _object_id  = item.value("object_id").toInt();
    _object_no  = item.value("object_no").toString();
    _image_id   = item.value("image_id").toInt();
    _image_list = item.value("image_list").toString();
    _task_id    = item.value("task_id").toInt();
    _slide_id   =item.value("slide_id").toInt();

    CellPrivate _celldata;
    _celldata._create_time  = item.value("create_time").toString();

    _celldata._id        = item.value("id").toInt();
    _celldata._oldType   = item.value("object_type_id").toInt();
    _celldata._category_id      = _celldata._oldType;
    _celldata._level     = item.value("level").toInt();
    _celldata._contours  = decodeCountours(item.value("contours").toArray());
    _celldata._contours_id  = item.value("contours_id").toString();
    _celldata._user_name = item.value("username").toString();
    _celldata._user_id   = item.value("user_id").toInt();
    _celldata._pixmap    = QPixmap();
    _celldata._action    = item.value("action").toInt();
    _celldata._deleted   = _celldata._action==3;
    // 注意, path字段是全路径名, 新增的字段, 可能不存在. 当不存在时为空.
    _celldata._path      = item.value("path").toString();
    _celldata._rect      = decodeOutline(_object_id, item.value("outlines").toString());

    _values.push_back(_celldata);

    return true;
}

void CellItem::loadContours(const QByteArray &data)
{
    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(data, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        ERROR() << "parse contours failed!";
        return;
    }
    for(auto item: doc.object().value("data").toArray())
    {
        auto point = item.toArray();
        this->_values.last()._contours.append(QPoint(point.at(0).toInt(), point.at(1).toInt() ));
    }
}

QPolygon CellItem::decodeCountours(const QJsonArray& array)
{
    QPolygon contours;
    std::transform(array.begin(), array.end(), std::back_inserter(contours),
                   [](const QJsonValue& value){
            auto point = value.toArray();
            return QPoint(point.at(0).toInt(), point.at(1).toInt());
    });
    return contours;
}

QRect CellItem::decodeOutline(int obj_id, const QString &outlines)
{

    // 数据库中使用了单引号, 需要转化成双引号再解析json
    QString tmp = outlines;
    tmp.replace('\'', '\"');
    QJsonParseError jsonError;
    QJsonDocument doc;
    doc = QJsonDocument::fromJson(tmp.toUtf8(), &jsonError);
    if( doc.isNull()){
        ERROR() << QString("cell outlines parse error. obj_id=%1, error is: %2").arg(obj_id).arg(jsonError.errorString());
        return QRect();
    }
    auto obj = doc.object();
    return QRect( obj.value("x").toInt(), obj.value("y").toInt(), obj.value("width").toInt(), obj.value("height").toInt() );
}

const CellPrivate& CellItem::value() const
{
    return _values.last();
}

CellPrivate& CellItem::value()
{
    return _values.last();
}

const QString CellItem::typeNo() const
{
    int type_id = this->type();
    if( this->_catalog.contains(type_id)){
        return this->_catalog.value(type_id).typeNo;
    }
    return "未知分类";
}

const QString CellItem::typeName() const
{
    int type_id = this->type();
    if( this->_catalog.contains(type_id)){
        return this->_catalog.value(type_id).cnName;
    }
    return "未知分类";
}


void CellItem::setCategoryId(int category_id)
{
    this->_values.last()._category_id = category_id;
}

int CellItem::getCategoryId() const
{
    return this->_values.last()._category_id;
}

QString CellItem::toString() const
{
    return QString("[CellItem] obj_id=%1, id=%2, catalog=%3, catalogId=%4")
            .arg(this->_object_id).arg(this->uid()).arg(typeName()).arg(this->type());
}

QString CellItem::showInfo() const
{
    return QString(QObject::tr("细胞ID=%1, 类别=%2").arg(this->uid()).arg(this->typeName()));
}


