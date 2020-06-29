#ifndef TASKITEM_H
#define TASKITEM_H
#include <QMetaType>
#include <QString>

struct SlideItemPrivate
{
    int     _uid;                //! 数据库中的记录ID, 来自字段id, 别的消息中也称为slide_id
    int     _slide_id;           // 玻片编号, 是uid的别名
    QString _slide_no;           //! 玻片的文本名称, 来自字段slide_no
    QString _assay_type_name;    //! 玻片类型的名称, 来自字段assay_name
    int     _assay_type_id;      //! 玻片类型的ID, 来自assay_type_id
    int     _image_count;        //! 玻片视野个数, 来自于image_count
    int     _image_doing;        //! 来自字段doing
    int     _image_done;         //! 完成的视野数, 核型使用, 来自done
    QString _case_number;        //! 来自case_number
    QString _create_time;        //! 扫描日期, 来自create_time
    int     _my_status;          //! 玻片的状态, 对应于state.
    int     _slide_status;       //! 玻片状态. 对应于status
    int     _device_id;          //! 来自字段device_id
    QString _device_sn;          //! 来自字段device_sn
    QString _device_type_name;   //! 来自字段device_type
    int     _cell_count;         //! 玻片中的细胞个数, 来自字段"cell_count"
    int     _level;              //! 来自字段level.  ( /api/slides/take_list命令不支持)
    QString _hospital;           //! 来自字段hospital
    QString _path;               //! 来自字段path (/api/slides/my/slides/in/tasks不支持)
};

class SlideItem
{
public:
    inline int slideId() const { return this->_data._slide_id; }
    inline QString slideNo() const { return this->_data._slide_no; }
    inline int assayTypeId() const { return this->_data._assay_type_id; }
    inline QString assayTypeName() const { return this->_data._assay_type_name; }
    inline int cellCount() const { return this->_data._cell_count; }
    inline int imageCount() const { return this->_data._image_count; }
    inline int doingCount() const { return this->_data._image_doing; }
    inline int doneCount() const { return this->_data._image_done; }
    inline QString caseNo() const { return this->_data._case_number; }
    inline QString createTime() const { return this->_data._create_time; }
    inline int taskStatus() const { return this->_data._my_status; }
    inline int slideStatus() const { return this->_data._slide_status; }
    inline int deviceTypeId() const { return this->_data._device_id; }
    inline QString deviceSN() const { return this->_data._device_sn; }
    inline QString deviceTypeName() const {return this->_data._device_type_name; }
public:

    //! 从/api/slides/my/slides/in/tasks的响应中获取数据, 用于查询我名下的任务(玻片)的信息
    void fromInTaskJson(const QJsonObject& obj);
    //! 从/api/slides/take_list的结果中获取数据, 用于查询待领取任务
    void fromTakeListJson(const QJsonObject&obj);
    //! 返回玻片的级别
    int     getLevel() const {return this->_data._level; }

    operator QString () {
        return QString("[TaskItem: uid=%1, id=%2, status=%3, type=%4, typeId=%5, count=%6]")
                .arg(_data._uid).arg(_data._slide_no).arg( _data._my_status).arg(_data._assay_type_name).arg(_data._assay_type_id).arg(_data._image_count);
    }
private:
    SlideItemPrivate    _data;
};



Q_DECLARE_METATYPE(SlideItem)
#endif // TASKITEM_H
