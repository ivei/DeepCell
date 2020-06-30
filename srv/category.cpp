#include <QJsonObject>
#include "category.h"
#include "commdefine.h"
#pragma execution_character_set("utf-8")



void CategoryItem::fromJson(const QJsonObject &obj)
{
    this->id    = obj.value("id").toInt();
    this->cnName= obj.value("category").toString();
    this->enName= obj.value("category_en").toString();
    this->typeNo= obj.value("object_type_no").toString();
    this->family= obj.value("family").toString("01"); // 兼容性处理, 如果不存在family字段, 就默认认为是01
    // 处理老数据库没有family字段的情况
    if( this->family=="01" && this->id>=25 && this->id<=49){
        this->family = "03";
    }
    else if(this->family=="01" && this->id>=51 )
    {
        this->family = "02";
    }
    else if(this->family=="01" && (this->id==24 || this->id==50 || this->id>54))
    {
        this->family    = "04";
    }
}

QString CategoryItem::getName(int language) const
{
    return (language==0) ? this->cnName : this->enName;
}


CategoryFamily filtFamily(const CategoryFamily &catalog, const QString &familyId)
{
    CategoryFamily result;
    QStringList rule;
    if( familyId==TCellFamily::FamilyHema || familyId==TCellFamily::FamilyMega )
    {
        rule << TCellFamily::FamilyHema << TCellFamily::FamilyMega;
    }
    else {
        rule << TCellFamily::FamilyKaryo;
    }
    for(auto iter=catalog.begin(); iter!=catalog.end(); ++iter)
    {
        if( rule.contains( iter.value().family)  )
        {
            result.insert(iter.key(), *iter);
        }

    }
    return result;
}


