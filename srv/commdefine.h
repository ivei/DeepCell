#ifndef COMMDEFINE_H
#define COMMDEFINE_H
#include <QString>
#include <QColor>
/** @file commdefine.h
 *  @brief  定义程序使用到的全局常量定义.
 *  @note
 *      在这里定义的常量必须满足两个要求:
 *      1. 被不止一个类所使用
 *      2. 使用的类之间没有直接的依赖关系.
 *      只有在这种情况下才将常量拿出来定义为公共.
 *      如果不满足上述条件, 则将常量定义在类中, 或者定义在类实现文件中(不共享)
 */

//! 玻片的检验类型代号. 来自数据库的定义.
namespace TAssayType {
    const int ALL_TYPE = 0;     //!  不存在的值
    const int BM_TYPE = 1;      //! 骨髓血
    const int PB_TYPE = 2;      //! 外周血
    const int TY_TYPE = 3;      //! 甲状腺=3
    const int KY_TYPE = 4;      //! 核型
    const int CP_TYPE = 5;      //! 连续 外周血=5
    const int TV_TYPE = 6;      //! 十倍 筛选=6
}

// 定义玻片状态.
namespace  TSlideStatus{
    const int SLIDE_ALL   = 0;     //! 类型不限. 数据库中实际不存在
    const int SLIDE_DOING = 1;     //!
    const int SLIDE_DONE  = 2;     //!
    const int SLIDE_UNDEF = -1;    //!

};

//! 玻片列表中的可能需要显示的字段名称.
enum TTaskViewColumnID{
    UID = 0,    //! 玻片ID
    SampleId,   //! 玻片号
    AssayType,  //! 玻片类型
    CaseNumber, //! 病历号
    CellCount,  //! 细胞个数
    ViewCount,  //! 视野个数
    CreateTime, //! 创建时间
    AssayTypeId,//! 玻片类型ID
    DeviceType, //! 设备类型名称
    DeviceTypeId,   //! 设备类型
    MyStatus,       //! 任务状态(我的任务)
    SlideStatus,    //! 玻片状态
    ColumnEnd
};

//! 定义细胞/核型的顶层分类.
namespace TCellFamily {
    const QString FamilyHema{"01"};     //! 白细胞
    const QString FamilyMega{"02"};     //! 巨核细胞
    const QString FamilyKaryo{"03"};    //! 核型
};

const int MAX_PAGE_SIZE = 500;   //! 分页查询时一页的最大条数.
const int FetchInterval = 10;

//! 用于显示的瓦片的尺寸. 目前瓦片固定为256x256, 因此只使用一个TILE_SIZE来标识长和宽.
const int TILE_SIZE = 256;

//! 打开玻片的进度控制. 总的步骤
//! 包括:
//! getTaskResult:
//!     1.读取字典, 2.读视野信息(一条消息), 3.读细胞信息(一条或多条并发) 4.下载视野缩略图(多条消息并发)
//!     5. 下载细胞图像(多条消息并发)
namespace ProgressDefine {
    enum PROGRESS
    {
        Init = 0,
        ReadCatalog,
        ReadImage,
        ReadCell,
        ReadImagePrev,
        ReadCellImage,
        SetCatalog,     // 设置分类视图
        InitShowPage,
        Buddy = InitShowPage
    };
}

namespace PALATTE {
    //! 定义调色板. https://www.sojson.com/web/page.html
    struct ColorPair{
        QString firstColor;
        QString secondColor;
    };

    const QList<ColorPair> ColorPalette{
         {"#000000", "#FFFFFF"}     // 黑色, black    白色, white
        ,{"#00FFFF", "#000000"}     // 青色 cyan      黑色, black
        ,{"#008B8B", "#FFFFFF"}     // 暗青色	darkcyan    白色, white
        ,{"#FF0000", "#FFFFFF"}     // 红色	red     白色, white
        ,{"#8B0000", "#FFFFFF"}     // 暗红色	darkred : 白色, white
        ,{"#FF00FF", "#FFFFFF"}     // 紫红色fuchsia(magenta) : 白色, white
        ,{"#8B008B", "#FFFFFF"}     // 暗洋红	darkmagenta : 白色, white
        ,{"#00FF00", "#000000"}     // 绿色 green  :  黑色 black
        ,{"#006400", "#FFFFFF"}     // 暗绿色	darkgreen : 白色white
        ,{"#FFFF00", "#000000"}     // 黄色	yellow  :  黑色 black
        ,{"#8B8B00", "#000000"}     // 深黄色 darkyellow  :  黑色 black
        ,{"#0000FF", "#FFFFFF"}     // 蓝色   blue
        ,{"#00008B", "#FFFFFF"}     // 暗蓝色	darkblue
        ,{"#808080", "#FFFFFF"}     // 灰色	gray
        ,{"#A9A9A9", "#FFFFFF"}     // 暗灰色	darkgray
        ,{"#D3D3D3", "#000000"}     // 浅灰	lightgray
        ,{"#A52A2A", "#FFFFFF"}     // 褐玫瑰红	brown
        ,{"#996600", "#FFFFFF"}     // 褐色
        ,{"#D2691E", "#FFFFFF"}     // 巧可力色	chocolate
        ,{"#FF1493", "#FFFFFF"}     // 深粉红色	deeppink
        ,{"#FFC0CB", "#000000"}     // 粉红色	pink
        ,{"#4B0082", "#FFFFFF"}     // 靛青色	indigo
        ,{"#8A2BE2", "#FFFFFF"}     // 紫罗兰色	blueviolet
        ,{"#FF4500", "#FFFFFF"}     // 红橙色	orangered
        ,{"#FF8C00", "#FFFFFF"}     // 暗桔黄色	darkorange
        ,{"#B8860B", "#FFFFFF"}     // 暗金黄色	darkgoldenrod
        ,{"#FFD700", "#000000"}     // 金色	gold
        ,{"#FFFFE0", "#000000"}     // 亮黄色	lightyellow
        ,{"#808000", "#000000"}     // 橄榄色	olive
        ,{"#008080", "#FFFFFF"}     // 水鸭色	green(teal)
        ,{"#00FF7F", "#000000"}     // 春绿色	springgreen
        ,{"#228B22", "#FFFFFF"}     // 森林绿	forestgreen
        ,{"#7FFFD4", "#000000"}     // 碧绿色	aquamarine
        ,{"#4169E1", "#FFFFFF"}     // 皇家蓝	royalblue
        ,{"#708090", "#FFFFFF"}     // 灰石色	slategray
        ,{"#FFFFFF", "#000000"}       // 白色, white
    };

}

#endif // COMMDEFINE_H
