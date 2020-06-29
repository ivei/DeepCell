#include <QDesktopServices>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPixmap>
#include <QProgressDialog>
#include <QSplashScreen>
#include <QSplitter>
#include <QUndoStack>
#include <QUrl>
#include "../frame/simpleprogressdlg.h"
#include "../frame/splashwndext.h"
#include "../srv/afxapp.h"
#include "../srv/categoryitem.h"
#include "../srv/cellitem.h"
#include "../srv/commdefine.h"
#include "../srv/dbgutility.h"
#include "../srv/deeplabel.h"
#include "../srv/deepservice.h"
#include "../srv/slideitem.h"
#include "../srv/taskresult.h"
#include "../srv/utilities.h"

#include "addcommand.h"
#include "analyzerwindow.h"
#include "catalogview.h"
#include "cellshowpage.h"
#include "celllistmodel.h"
#include "changetypecommand.h"
#include "deletecommand.h"
#include "imagedialog.h"
#include "imageview.h"
#include "reportdialog.h"
#include "reportdlg.h"

#include "ui_analyzerwindow.h"

#pragma execution_character_set("utf-8")

AnalyzerWindow::AnalyzerWindow(QWidget *parent) :
    QMainWindow(parent, Qt::WindowMinMaxButtonsHint| Qt::WindowCloseButtonHint),
    ui(new Ui::AnalyzerWindow),
    splitter(new QSplitter(this)),
    catalogView(new CatalogView(this)),
#ifdef CELL_SHOW_REF
    cellShowPage(new CellShowPage(CellShowPage::PBMode, this)),
#else
    tiledView(new TiledView(this)),
    galleryView(new GalleryView(this)),
#endif
    undoStack(new QUndoStack(this))
{
    ui->setupUi(this);
    auto layout = new QHBoxLayout(ui->centralwidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);
    splitter->addWidget(catalogView);
#ifdef CELL_SHOW_REF
    splitter->addWidget(cellShowPage);
#else
    splitter->addWidget(tiledView);
    splitter->addWidget(galleryView);
#endif
    // 调整细胞视图和大图视图的比例, 改为1:2.
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    splitter->setStretchFactor(2, 2);
    ui->centralwidget->setLayout(layout);
    ui->actionHiden->setChecked(true);  // 默认显示大图区域
#ifdef CELL_SHOW_REF
    cellShowPage->setVisible(false);
#else
    tiledView->setVisible(false);
    galleryView->setVisible(false);
#endif
    auto undo = undoStack->createUndoAction(this, "&Undo");
    undo->setIcon(QIcon(":/icons/undo.png"));
    undo->setShortcuts(QKeySequence::Undo);
    ui->toolBar->addAction(undo);
    auto redo = undoStack->createRedoAction(this, "&Redo");
    redo->setIcon(QIcon(":/icons/redo.png"));
    redo->setShortcuts(QKeySequence::Undo);
    ui->toolBar->addAction(redo);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionReport);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionMarkCell);
    ui->actionMarkCell->setChecked(true);
    ui->toolBar->addAction(ui->actionshowContour);
    ui->actionshowContour->setChecked(false);

    setupStatusBar();
}

AnalyzerWindow::~AnalyzerWindow()
{
    delete ui;
}

/**
 * @brief 初始化函数
 * @param provider
 * @param slide     玻片信息. 实际上可能只用了玻片ID一个值.
 */
void AnalyzerWindow::initialize(DeepLabel *provider, const SlideItem& slide)
{
    qint64 t0, t1, t2, te;
#if 0
    QSplashScreen splash(QPixmap(":/images/splash.jpg"), Qt::WindowStaysOnTopHint);
    splash.show();
    splash.showMessage(QSplashScreen::tr("正在启动中，请稍后."), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
#else
    SplashWndExt splash(QPixmap(":/images/splash.jpg"));
    connect(provider, &DeepLabel::sigProgress, &splash, &SplashWndExt::onSigProgress);
    connect(this, &AnalyzerWindow::sigProgress, &splash, &SplashWndExt::onSigProgress);
    splash.show();
#endif
    TRACE() << "slide_id is: " << slide.slideId() << "assay type is: " << slide.assayTypeName();
    this->setWindowTitle(slide.slideNo());
    // 获取本玻片下的视野和细胞的信息.
    START_CLOCK("Start clock...");

    t0 = t1 = GET_CLOCK();
    auto result = provider->getTaskResultEx(slide);
    t2 = GET_CLOCK();

    CLOCK() << QString("getTaskResult consume %1").arg(t2-t1);

    this->catalogDirectory = result.catalogs();

    displayStatusBar(AnalyzerWindow::DisplayCellNum, result.cellList().size());
    displayStatusBar(AnalyzerWindow::DisplayImageNum, result.imageList().size());
    displayStatusBar(AnalyzerWindow::DisplayImageType, result.imageList().first().imageTypeText());
    displayStatusBar(AnalyzerWindow::DisplayImageName, result.imageList().first().imageNo());
    displayStatusBar(AnalyzerWindow::DisplaySlideType, slide.assayTypeName());
    displayStatusBar(AnalyzerWindow::DisplaySlideName, slide.slideNo());
    displayStatusBar(AnalyzerWindow::DisplayUsername, provider->getUserName());

    t1 = GET_CLOCK();
    catalogView->setContent(result.catalogs(), result.cellList());
    emit sigProgress(ProgressDefine::SetCatalog, ProgressDefine::Buddy, tr("初始化视野..."));
    t2 = GET_CLOCK();
    CLOCK() << QString("setContent consume %1").arg(t2-t1);

    // 在此定义工具栏上面的按钮行为.
    // 删除一个细胞
    connect(catalogView, &CatalogView::cellDeleted, this, &AnalyzerWindow::onCellDeleted);
    // 修改细胞类型
    connect(catalogView, &CatalogView::cellTypeChanged, this, &AnalyzerWindow::onCellTypeChanged);
    // 显示细胞分类
    connect(ui->actionExpand, &QAction::triggered, catalogView, &CatalogView::showAll);
    // 折叠细胞分类
    connect(ui->actionCollapse, &QAction::triggered, catalogView, &CatalogView::hideAll);
    // 保存到服务器
    connect(ui->actionSave, &QAction::triggered, [this, slide, provider](){
        SimpleProgressDlg   progressDialog(this);
        progressDialog.show();
        undoStack->clear();
        auto items = catalogView->sourceModel()->getSource();
        catalogView->sourceModel()->sync(provider->saveResult(slide, items));
        progressDialog.close();
    });
    // 提交审核结果
    connect(ui->actionCommit,&QAction::triggered, [this, slide, provider](){
        SimpleProgressDlg   progressDialog(this);
        progressDialog.show();
        undoStack->clear();
        auto items = catalogView->sourceModel()->getSource();
        catalogView->sourceModel()->sync(provider->saveResult(slide, items));
        provider->commitResult(slide.slideId());
        progressDialog.close();
    });
    // 出报告
    connect(ui->actionReport, &QAction::triggered, [this, provider, slide](){
        TRACE() << "slide_id is: " <<  slide.slideId() << "slide_no is: " << slide.slideNo();
        ReportDlg dlg(this);
        dlg.setWindowTitle(QString("%1_%2").arg(tr("诊断结论")).arg(slide.slideNo()));
        dlg.setData(provider, slide);
        if(dlg.exec() == QDialog::Accepted)
        {
            QString pdfFileName= DeepService::getReportFilePath(provider->getUserName(), slide.slideNo());
            QDesktopServices::openUrl(QUrl::fromLocalFile(pdfFileName));
        }
    });

    // 抓图
    connect(ui->actionCapture, &QAction::triggered, this, [this](){
        ImageDialog(screenshots, this).exec();
    });

    // 设置CellShowPage的工作模式
    this->cellShowPage->setWorkMode(slide.assayTypeId()==TAssayType::BM_TYPE ? CellShowPage::BMMode : CellShowPage::PBMode);
    // 添加细胞.
    connect( cellShowPage->view(), &ImageView::objectAdded, [this, &slide](const QPolygonF& polygon){
        IF_RETURN(polygon.size()<3);    // 少于三个点, 形不成一个区域



        // 从视图上面拷贝出细胞的图片
        auto roi = polygon.boundingRect().toRect();
        QImage image = cellShowPage->capture(roi);
        IF_RETURN(image.isNull());
        QString cell_family = this->cellShowPage->view()->getCellFamily();
        auto cell_catalog = filtFamily(this->catalogDirectory, cell_family);

        auto contour = findCell(image, polygon.translated(-roi.topLeft()).toPolygon());

        CellPrivate data;
        data._category_id = 1;  // 新增的细胞类别都是1(未分类)
//        data._category_name = this->catalogDirectory.value(1).cnName;
//        data._category_no   = this->catalogDirectory.value(1).typeNo;
        data._level         = slide.getLevel();
        data._contours = contour.translated(roi.topLeft());
        auto center = data._contours.boundingRect().center();
        data._pixmap = QPixmap::fromImage(cellShowPage->capture(QRect(center.x()-250, center.y()-250, 500, 500)));
        data._pixmap = data._pixmap.scaled(QSize(200,200));
        data._rect   = data._contours.boundingRect();

        CellItem item;
        item.setValue(data);
        item.setImageId(cellShowPage->currentImageData().id());
        item.setTaskId(cellShowPage->currentImageData().taskId());
        item.setCatalog(cell_catalog);
        TRACE() << "new Cell Object: " << item.toString();
        onCellAdded(item);

        ui->actionshowContour->setChecked(true);
        ui->actionMarkCell->setChecked(true);
    });
    // 抓图
    connect(cellShowPage->view(), &ImageView::captured, this, &AnalyzerWindow::onCaptured);

    //cellShowPage->initialize(result, provider);

    connect(catalogView->sourceModel(), &CellListModel::cellAdded, cellShowPage, &CellShowPage::onCellAdded);
    connect(catalogView->sourceModel(), &CellListModel::cellRemoved, cellShowPage, &CellShowPage::onCellDeleted);
    connect(catalogView->sourceModel(), &CellListModel::cellTypeChanged, cellShowPage, &CellShowPage::onCellTypeChanged);
    // 选择了细胞的映射.
    connect(catalogView, &CatalogView::cellSelected, cellShowPage, &CellShowPage::onCellSelected);
    connect(ui->actionHiden, &QAction::triggered, this, [this](bool checked){cellShowPage->setVisible(checked);});
    connect(ui->actionMarkCell, &QAction::toggled, cellShowPage, &CellShowPage::onCellShowMarkChanged);
    connect(ui->actionshowContour, &QAction::toggled, cellShowPage, &CellShowPage::onCellShowCounterChanged);


    // 来自图片视图的双击事件. 在ImageView中双击细胞, 在CatalogView中需要更新选中.
//    connect(cellShowPage->view(), &ImageView::doubleClicked, [](int cellId){
//        TRACE() << "double clicked at cell " << cellId;
//    });
    connect(cellShowPage->view(), &ImageView::doubleClicked, catalogView, &CatalogView::onSigSelectCell);
    // 映射来自CellShowPage的删除细胞消息
    // 暂时未支持Undo/Redo框架. 以后考虑
    connect(cellShowPage->view(), &ImageView::sigCellDeleted, catalogView, &CatalogView::onSigCellDelete);

    // 映射来自CellShowPage的修改细胞类型信息
    // 暂时未使用Undo/Redo. 以后再考虑
    connect(cellShowPage->view(), &ImageView::sigCellTypeChanged, catalogView, &CatalogView::onSigCellTypeChanged);

    //! 状态栏的显示
    connect(this->cellShowPage, &CellShowPage::sigNotifyMsg, this, &AnalyzerWindow:: displayStatusBar);
    this->cellShowPage->setVisible(true);

    t1 = GET_CLOCK();
    cellShowPage->initialize(result, provider);
    emit sigProgress(ProgressDefine::InitShowPage, ProgressDefine::Buddy, tr("完成"));
    t2 = GET_CLOCK();
    CLOCK() << QString("cellShowPage::initialize consume %1").arg(t2-t1);

    showMaximized();
    splash.finish(this);

    te = GET_CLOCK();
    CLOCK() << QString("AnalyzerWindow::initialize() consume %1").arg(te-t0);
}

void AnalyzerWindow::exec()
{
    QEventLoop loop;
    connect(this, &AnalyzerWindow::close, &loop, &QEventLoop::quit);
    loop.exec();
}

void AnalyzerWindow::onCellDeleted(const CellItem& item)
{
    undoStack->push(new DeleteCommand(item, catalogView->sourceModel()));
}

void AnalyzerWindow::onCellTypeChanged(const CellItem& item, int type)
{
    undoStack->push(new ChangeTypeCommand(item, catalogView->sourceModel(), type));
}

void AnalyzerWindow::onCellAdded(const CellItem& item)
{
    undoStack->push(new AddCommand(item, catalogView->sourceModel()));
}

void AnalyzerWindow::onCaptured(const QPixmap& pixmap)
{
    screenshots.append(pixmap);
}



void AnalyzerWindow::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e)
    emit close();
    QMainWindow::closeEvent(e);
}

void AnalyzerWindow::setupStatusBar()
{
    QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);

    // 显示当前用户名信息
    statusBar()->addPermanentWidget(new QLabel(tr("当前用户")));
    this->userNameInd = new QLineEdit;
    this->userNameInd->setReadOnly(true);
    this->userNameInd->setFrame(true);
    this->userNameInd->setSizePolicy(sizePolicy2);
    this->userNameInd->setFixedWidth(50);
    statusBar()->addPermanentWidget(this->userNameInd);
    // 显示玻片名称
    statusBar()->addPermanentWidget(new QLabel(tr("玻片名称")));
    this->slideNameInd = new QLineEdit;
    this->slideNameInd->setReadOnly(true);
    this->slideNameInd->setSizePolicy(sizePolicy1);
    this->slideNameInd->setMinimumWidth(80);
    statusBar()->addPermanentWidget(this->slideNameInd);

    // 显示玻片类别
    statusBar()->addPermanentWidget(new QLabel(tr("玻片类别")));
    this->slideTypeInd = new QLineEdit;
    this->slideTypeInd->setReadOnly(true);
    this->slideTypeInd->setSizePolicy(sizePolicy2);
    this->slideTypeInd->setFixedWidth(50);
    statusBar()->addPermanentWidget(this->slideTypeInd);

    // 显示视野个数信息
    statusBar()->addPermanentWidget(new QLabel(tr("视野数")));
    this->imageNumInd  = new QLCDNumber(4);
    statusBar()->addPermanentWidget(imageNumInd);

    // 显示当前视野类型
    statusBar()->addPermanentWidget(new QLabel(tr("当前视野类别")));
    this->imageTypeInd = new QLineEdit;
    this->imageTypeInd->setReadOnly(true);
    this->imageTypeInd->setSizePolicy(sizePolicy1);
    //this->imageTypeInd->setMinimumWidth(10);
    statusBar()->addPermanentWidget(imageTypeInd);

    // 显示当前视野名称
    statusBar()->addPermanentWidget(new QLabel(tr("当前视野")));
    this->imageNameInd = new QLineEdit;
    this->imageNameInd->setReadOnly(true);
    this->imageNameInd->setSizePolicy(sizePolicy1);
    this->imageNameInd->setMinimumWidth(150);
    statusBar()->addPermanentWidget(this->imageNameInd);

    // 显示细胞个数
    statusBar()->addPermanentWidget(new QLabel(tr("细胞总数")));
    this->cellNumInd = new QLCDNumber(4);
    statusBar()->addPermanentWidget(this->cellNumInd);



}

void AnalyzerWindow:: displayStatusBar(int type, const QVariant &v)
{
    switch (type) {
    case DisplayNormal:
        this->statusBar()->showMessage(v.toString(), 5000);
        break;
    case DisplayCellNum: // 细胞数
        this->cellNumInd->display(v.toInt());
        break;
    case DisplayCellSelect:
        this->statusBar()->showMessage(v.toString(), 5000);
        break;
    case DisplayImageType:
        this->imageTypeInd->setText(v.toString());
        break;
    case DisplayImageName:  // 显示当前视野名称
        this->imageNameInd->setText(v.toString());
        break;
    case DisplayImageNum: // 视野数
        this->imageNumInd->display(v.toInt());
        break;
    case DisplaySlideType:
        this->slideTypeInd->setText(v.toString());
        break;
    case DisplaySlideName:
        this->slideNameInd->setText(v.toString());
        break;
    case DisplayUsername:
        this->userNameInd->setText(v.toString());
        break;
    }
}


