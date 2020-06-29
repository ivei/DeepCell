#ifndef CELLSHOWPAGE_H
#define CELLSHOWPAGE_H

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QListView>
#include <QListWidget>
#include <QWidget>

#include "../srv/categoryitem.h"
#include "../srv/cellitem.h"
#include "../srv/imageitem.h"

class TaskResult;
class DeepLabel;
class CellItem;
class Categoryitem;
class ImageView;
class TileProvider;


/**
 * @brief 在这里保存每个视野对应的存储信息的集合.
 */
struct ImageSetItem
{
    QString         key;
    // QGraphicsItem   *graphicsItem;  //! Image对应的图项的指针
    QList<QGraphicsItem *> showItemList;
    QGraphicsScene  *scene;         //! Image所在的视野的指针
    ImageItem       data;           //! Image对应的ImageItem数据
    QPixmap         thumbPixmap;    //! 缩略图的Pixmap
    ImageSetItem(){};
    ~ImageSetItem(){};
};

struct CellSetItem
{
    int             cellId;     // CellId
    int             imageId;    // 对应的ImageID. 为了简单
    QString         imagePath;  // 对应的Image的path, 为了方便和ImageSet关联使用.
    QGraphicsItem   *showItem;
    QGraphicsScene  *scene;
    CellItem        data;
};

class CellShowPage : public QWidget
{
    Q_OBJECT
public:
    enum WorkMode{
        BMMode,
        PBMode
    };

    //const int TILE_SIZE = 256;  //! 瓦片的尺寸.
    enum NotifyId
    {
         NotifyNormal        = 0    //! 通用信息.
        ,NotifyCellNumber    = 1    //! 细胞数
        ,NotifyCellSelect    = 2    //! 选择的细胞
        ,NotifyImageType     = 3    //! 选择的视野的类别
        ,NotifyImageName
    };

    explicit CellShowPage(CellShowPage::WorkMode workmode, QWidget *parent = nullptr);
    ~CellShowPage() override;
    void setWorkMode(CellShowPage::WorkMode workmode);
    CellShowPage::WorkMode workMode() const ;
    void initialize(const TaskResult &result, DeepLabel* handler);
    virtual QImage capture(const QRect& rect);
    //! 返回视野视图的指针
    ImageView *view() { return this->graphicsView;}
    //! 返回当前视野视图对应的Scene.
    QGraphicsScene *scene(){ return this->graphicsScene; }

    //! 返回当前显示的视野的ImageItem数据.
    const ImageItem& currentImageData() const;

    bool cellMarkShow() const;
    void setCellMarkShow(bool show);
    bool cellContourShow() const;
    void setCellContourShow(bool show);

protected:
    void initializeNew(const TaskResult& result, DeepLabel* handler);

signals:
    //! 在视图中修改了细胞类型的通知信号.

    // CellShowPage需要处理的信号
public slots:
    //! 外部传进来的信号的处理.
    //!  外部新增细胞
    virtual void onCellAdded(const CellItem& item);
    //!  外部选择细胞(需要更新视图)
    virtual void onCellSelected(const CellItem& item);
    //!  外部删除细胞(需要更新视图)
    void deleteCell(int cellId);
    virtual void onCellDeleted(const CellItem& item);
    //! 外部修改了细胞类型
    //void changeCellType(int cellId, int newType);
    virtual void onCellTypeChanged(const CellItem&, int type);
    //! 外部修改了细胞的显示方式
    virtual void onCellShowMarkChanged(bool isShow);
    virtual void onCellShowCounterChanged(bool);

    // 内部传出来的信号的处理.
    void onCurrentImageChanged(QListWidgetItem* current, QListWidgetItem* previous);


signals:    // 发出去的信号, 用户在本视图中进行的操作.
    void sigNotifyMsg(int type, const QVariant& value);

private:
    //! 界面元素创建函数. 本类没有使用Qt Designer, 而是手工创建界面.
    void setupUI();
    //! 切换视野显示. 每个事业对应于一个Scene, 通过视野的image_path来唯一标识.
    void switchImage(const QString& new_path);
    //! 在视野的视图中居中显示指定的细胞.
    void centerCell(QGraphicsItem* cell);
private:


    CellShowPage::WorkMode _workMode;       //!
    QMap<QString, ImageSetItem> imageSet;
    QMap<int, CellSetItem>      cellSet;
    //! 当前处理的视图的数据 ImageItem. 临时保存
    ImageItem   image;
    DeepLabel   *handler;
    TileProvider * provider;


    QMap<int, CategoryItem> categorys;
    bool _cellMarkShow;
    bool _cellContourShow;

    // 图像界面元素
    ImageView       *graphicsView;
    QGraphicsScene  *graphicsScene;


    QListWidget     *listWidget;
    QListView       *listView;
    QHBoxLayout     *mainLayout;

};

#endif // CELLSHOWPAGE_H
