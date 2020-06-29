#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QString>
#include <QSize>
#include <QPixmap>

class QJsonObject;
class ImageItem
{
public:
    //! Image的类型
    enum{
        TypeTile100x = 1,
        TypeNormal,
        TypeTile10x,
        TypeMega
    };
    //! 从/api/tasks/mylist 中获取结果
    bool fromJson(const QJsonObject&);

    //! 返回Image的image_id
    inline int id() const { return _image_id; }
    inline int imageType() const {return _image_type; }
    inline QString imageName() const { return _image_name; }
    inline QString imageNo() const { return _image_no; }
    inline QString imagePath() const { return _image_path; }
    inline QSize imageSize() const { return _image_size; }
    //! 是否是瓦片显示方式.
    inline bool isTile() const {return _image_type!=TypeNormal; }
    inline int level() const {return _task_level; }
    inline int slideId() const { return _slide_id; }
    inline QString slideNo() const { return _slide_no; }
    inline QString slidePath() const { return _slide_path; }
    inline QString suffix() const { return _suffix; }
    //! 返回ImageItem所属的TaskID
    inline int taskId() const { return _taskId; }

    //! 缩略图的路径
    inline QString thumbnail() const { return _thumbnail; }
    QString tileBaseDir() const;
    inline QSize tileSize() const { return _tile_size; }

    void setPrevPixmap(const QPixmap& pixmap){ this->_thumbPixmap = pixmap; }
    const QPixmap& prevPixmap() const { return this->_thumbPixmap; }

    QString imageTypeText() const;
    static QString makeTileFilePath(const QString& base_path, int level, int x, int y, const QString& sufix);


private:
    int     _image_id;
    int      _taskId;
    QString _image_name;
    QString _image_no;
    int     _image_type;
    QString _suffix;    //! 图像文件的后缀名
    QString _slide_path;      //! slides里面的path, 服务器的路径.
    QString _image_path;       //! images里面的path. 和_path相比通常是相同的.

    QString _thumbnail; //! 缩略图的路径

    int     _slide_id;
    QString _slide_no;
    int     _cur_level;     //! 针对视野的data/images/level.
    int     _task_level;    //! 针对自己的data/level, 建议使用这个.
    bool    _saved;
    int     _my_status;
    bool    _finished;

    QSize   _image_size;
    QSize   _tile_size;

    int     _user_id;
    QString _user_name;
    int     _user_type;     //! 等同于cur_level

    QPixmap _thumbPixmap;
};

#endif // IMAGEITEM_H
