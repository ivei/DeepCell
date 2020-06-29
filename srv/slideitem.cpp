#include "slideitem.h"
#include <QJsonObject>

#pragma execution_character_set("utf-8")

void SlideItem::fromInTaskJson(const QJsonObject &obj)
{
    this->_data._uid       = obj.value("id").toInt();
    this->_data._slide_id  = this->_data._uid;
    this->_data._slide_no  = obj.value("slide_no").toString();
    this->_data._assay_type_name   = obj.value("assay_name").toString();
    this->_data._assay_type_id     = obj.value("assay_type_id").toInt();
    this->_data._image_count   = obj.value("image_count").toInt();
    this->_data._image_doing   = obj.value("doing").toInt();
    this->_data._image_done    = obj.value("done").toInt();
    this->_data._case_number   = obj.value("case_number").toString();
    this->_data._create_time   = obj.value("create_time").toString();
    this->_data._my_status     = obj.value("state").toInt();
    this->_data._slide_status  = obj.value("status").toInt();
    this->_data._device_id     = obj.value("device_id").toInt();
    this->_data._device_sn     = obj.value("device_sn").toString();
    this->_data._device_type_name  = obj.value("device_type").toString();
    this->_data._cell_count    = obj.value("cell_count").toInt(-1);
    this->_data._level        = obj.value("level").toInt();
    this->_data._hospital     = obj.value("hospital").toString();
    this->_data._path         = "";   // 不支持
}

void SlideItem::fromTakeListJson(const QJsonObject &obj)
{
    this->_data._uid = obj.value("id").toInt();
    this->_data._slide_id      = this->_data._uid;
    this->_data._slide_no      = obj.value("slide_no").toString();
    this->_data._assay_type_name   = obj.value("assay_types").toObject().value("assay_name").toString();
    this->_data._assay_type_id     = obj.value("assay_types").toObject().value("id").toInt();
    this->_data._image_count   = obj.value("image_count").toInt();
    this->_data._image_doing   = -1;
    this->_data._image_done    = -1;
    this->_data._case_number   = obj.value("case_number").toString();
    this->_data._create_time   = obj.value("create_time").toString();
    this->_data._my_status     = -1;       // 没有定义.
    this->_data._slide_status  = obj.value("status").toInt();
    this->_data._device_id     = obj.value("device_id").toInt();
    this->_data._device_sn     = obj.value("devices").toObject().value("sn").toString();
    this->_data._device_type_name  = obj.value("devices").toObject().value("device_type").toString();
    this->_data._cell_count    = obj.value("cell_count").toInt(-1);
    this->_data._level        = -1;
    this->_data._hospital     = obj.value("hospital").toString();
    this->_data._path         = obj.value("path").toString();
}
