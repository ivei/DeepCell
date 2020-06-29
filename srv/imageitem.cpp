#include "imageitem.h"
#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include "dbgutility.h"

#pragma execution_character_set("utf-8")

//! 统一的从Image路径得到预览图路径的方法. 不管是文件名还是路径名, 都是如此.
QString image2preview(const QString& image_path)
{
    auto path = image_path;
    auto pos = path.lastIndexOf("/data/");
    if( pos>=0){
        path.replace(pos, 6, "/preview/");
    }
    return path;
}

bool ImageItem::fromJson(const QJsonObject& obj)
{
    auto image_obj = obj.value("images").toObject();
    auto slide_obj = obj.value("slides").toObject();
    auto user_obj  = obj.value("users").toObject();

    _taskId = obj.value("id").toInt();
    _image_id = image_obj.value("id").toInt();  // 或者直接obj.value("image_id").toInt()
    _image_name = image_obj.value("name").toString();
    _image_no   = image_obj.value("image_no").toString();   // name去掉了文件后缀名
    _image_type = image_obj.value("image_type").toInt();
    _suffix     = _image_name.split('.').last();

    _slide_path = slide_obj.value("path").toString();
    _image_path = image_obj.value("path").toString();

    // 利用图片尺寸和瓦片尺寸是否相等来判断是否是骨髓的瓦片. 如果相等, 表示不是瓦片.
    _image_size = QSize(image_obj.value("global_width").toInt(), image_obj.value("global_height").toInt());
    _tile_size  = QSize(image_obj.value("tile_width").toInt(), image_obj.value("tile_height").toInt());


    // 识别老的瓦片格式. 老的瓦片只有100倍, 路径中不会出现/100x
    if( _image_type==ImageItem::TypeTile100x && _image_path.indexOf("/100x")<0)
    {
        _thumbnail = QString("%1/dzi/8/0_0.jpg").arg(_slide_path);
    }
    else // 外周血或新的骨髓方式
    {
        if( _image_type==ImageItem::TypeNormal){
            _thumbnail = _image_path;
        }
        else {
            _thumbnail = _image_path + "/" + _image_name;
        }
        auto pos = _thumbnail.lastIndexOf("/data/");
        if( pos>0){
            _thumbnail.replace(pos, 6, "/preview/");
        }

    }

    _slide_id   = obj.value("slide_id").toInt();
    _slide_no   = slide_obj.value("slide_no").toString();
    _cur_level  = image_obj.value("curr_level").toInt();
    _task_level = obj.value("level").toInt();
    _saved      = (obj.value("saved").toInt()!=0);
    _my_status  = image_obj.value("status").toInt();
    _finished   = _my_status==2;

    _user_id    = user_obj.value("id").toInt();
    _user_name  = user_obj.value("realname").toString();
    _user_type  = user_obj.value("user_type").toInt();
    return true;
}

/**
 * @brief 构造Tile的根目录. 即从/level往前的路径.
 * @return
 * @note
 *  老数据格式中,  tile的格式为 slide_path / dzi/ level/ x_y.jpg
 *  新的数据格式中,tile的格式为  image_p/ath 中将/data/ 改为 /dzi/.
 *  或者说, slide_path /dzi/ 100x|10x|mega / level/ z_y.jpg
 *
 *  本函数兼容新老数据格式:
 *      老数据格式中只有外周血和骨髓, 因此通过检查是否有
 */
QString ImageItem::tileBaseDir() const
{
    //Q_ASSERT_X(this->imageType()==ImageItem::TypeNormal, "tileBaseDir", "this function can not used for PB images!");
    // 若找不到"/100x", 就认为是老的骨髓(不考虑10x, mega等场景)
    if(this->imageType()==ImageItem::TypeNormal)
    {
        ERROR() << "error! tileBaseDir() should only used for BM slide!";
        return QString();
    }
    else if( this->imageType()==ImageItem::TypeTile100x && _image_path.indexOf("/100x")<0){
        return _slide_path + "/dzi";
    }
    else {
        QString path = _image_path;
        auto pos = path.lastIndexOf("/data/");
        if( pos>=0){
            path.replace(pos, 6, "/dzi/");
        }
        return path;
    }
}

QString ImageItem::imageTypeText() const
{
    switch (this->imageType()) {
    case ImageItem::TypeNormal:
        return QObject::tr("外周血百倍图");
    case ImageItem::TypeTile100x:
        return QObject::tr("骨髓百倍图");
    case ImageItem::TypeTile10x:
        return QObject::tr("10倍预览图");
    case ImageItem::TypeMega:
        return QObject::tr("巨核细胞百倍图");
    }
    return QObject::tr("其他");
}

/**
 * @brief 构造瓦片的文件全路径. 放在这里便于集中控制.
 * @param base_path 基础路径名, 即tileBaseDir()的返回值
 * @param level
 * @param x
 * @param y
 * @param sufix
 * @return
 */
QString ImageItem::makeTileFilePath(const QString &base_path, int level, int x, int y, const QString &sufix)
{
    return QString("%1/%2/%3_%4.%5")
            .arg(base_path)
            .arg(level)
            .arg(x).arg(y)
            .arg(sufix);
}

