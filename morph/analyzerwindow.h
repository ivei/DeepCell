#ifndef ANALYZERWINDOW_H
#define ANALYZERWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QLabel>
#include <QLineEdit>
#include <QLCDNumber>
#include <QPlainTextEdit>
#define CELL_SHOW_REF

namespace Ui {
class AnalyzerWindow;
}
class CategoryItem;
class QSplitter;
class QUndoStack;
class CatalogView;
class TiledView;
class GalleryView;
class SlideItem;
class CellItem;
class DeepLabel;
class CategoryItem;
class CellShowPage;

class AnalyzerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnalyzerWindow(QWidget *parent = nullptr);
    ~AnalyzerWindow() override;
    void initialize(DeepLabel *provider, const SlideItem& task);
    void exec();
signals:
    void close();
    void sigProgress(int cur, int total, const QString& msg);
protected slots:
    void onCellDeleted(const CellItem&);
    void onCellTypeChanged(const CellItem&, int);
    void onCellAdded(const CellItem&);
    void onCaptured(const QPixmap&);
    //! 显示指定的信息
    void  displayStatusBar(int type, const QVariant& v);

protected:
    void closeEvent(QCloseEvent *e) override;
private:
    void setupStatusBar();

    enum {
          DisplayNormal = 0  // 通用显示
        , DisplayCellNum      //! 细胞数
        , DisplayCellSelect   //! 当前选择的细胞信息
        , DisplayImageType    //! 当前选择的视野类型
        , DisplayImageName    //!
        , DisplayImageNum     = 10  //! 视野个数
        , DisplaySlideType    //! 玻片类型
        , DisplaySlideName    //! 玻片名称
        , DisplayUsername     //! 用户名
        , DisplayNone
    };

private:
    Ui::AnalyzerWindow *ui;
    QSplitter *splitter;
    CatalogView *catalogView;
#ifdef CELL_SHOW_REF
    CellShowPage *cellShowPage;
#else
    TiledView *tiledView;       // 骨髓的视图
    GalleryView *galleryView;   // 外周血的视图
#endif
    QUndoStack *undoStack;

    QList<QPixmap> screenshots;
    QMap<int, CategoryItem> catalogDirectory;



    QLineEdit   *userNameInd;   //! 显示用户名
    QLineEdit   *slideNameInd;     //! 显示玻片名称
    QLineEdit   *slideTypeInd;     //! 显示 玻片类型
    QLCDNumber  *imageNumInd;   //! 显示视野个数
    QLineEdit   *imageTypeInd;  //! 当前视野的类型(100倍, 10倍, ...)
    QLineEdit   *imageNameInd;     //! 显示当前视野名称
    QLCDNumber  *cellNumInd;    //! 显示细胞个数

 };

#endif // ANALYZERWINDOW_H
