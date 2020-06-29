#ifndef DEEPLABEL_H
#define DEEPLABEL_H
#include "deeplabelreply.h"
#include "datalist.h"
#include <QObject>
#include <QVariantMap>
#include <QList>
#include "commdefine.h"



class QNetworkAccessManager;
class QNetworkRequest;
class DeepLabelPrivate;
class SlideItem;
class ImageItem;
class CellItem;
class TaskResult;
class CategoryItem;
class QPolygon;


class DeepLabel: public QObject
{
    Q_OBJECT
public:
    // 支持多线程并发的接口,
    //! 支持并发和缓存的下载接口. 内部创建QNetworkAccessManager.
    QByteArray downloadDataEx(const QString& path_name, const QByteArray &token=false);
    TaskResult getTaskResultEx(const SlideItem& slide);
    DeepLabelReply *download(QNetworkAccessManager* manager, const QString& path, const QByteArray& token);
    // 用于单线程的接口
    explicit DeepLabel(QObject* parent=nullptr);
    ~DeepLabel();
    void setHost(const QString& host, int port=-1);
    void setCredential(const QString& username, const QString&password, const QString& userType="user");

    QString login(bool* success=nullptr);
    //! 调用/api/slides/my/slides/in/tasks 查询"我的任务", 返回"我"名下的Slide信息列表
    DataList<SlideItem> getTaskList(int start, int count);
    //! 调用/api/slides/take_list查询我可以领取的玻片列表
    DataList<SlideItem> getUnassignedTaskList(int start, int count);

    void takeTask(int uid);
    void untakeTask(int uid);

    //! 获取细胞轮廓的同步接口
    QPolygon getCellContours(const QString& contour_id);
    //! 获取细胞轮廓的异步接口
    DeepLabelReply* fetchCellContours(const QString& contour_id);

    DeepLabelReply *download(const QString& path);
    QByteArray downloadData(const QString& path, bool isthrow=false);
    //! 支持cache的下载. 如果cache中有数据, 就直接使用. cache的文件名加上.dat.

    //! 获取Image信息, 不使用缓存
    QList<ImageItem> getImageList(const SlideItem& slide);
    //! 获取玻片中的Image信息, 使用缓存机制
    QList<ImageItem> getImageListEx(const SlideItem& slide);

    //! 获取一个玻片下的所有的action列表.
    QList<CellItem> getSlideCells(const SlideItem& slide, const QString& username, int mode);



    //! 形态下, 获取一个Slide下的所有任务的信息.
    TaskResult getTaskResult(int slideId, int assayTypeId);

    QList<CellItem> saveResult(const SlideItem&, const QList<CellItem>&);

    bool commitResult(int slide_id);

    void delCell(int cell_id, int level);
    void modifyCellType(int cellId, int cellType, int level);
    CellItem addCell(int slideId, const CellItem& cell);

    QMap<int, QString> getTypes();
    QMap<int, CategoryItem>  getCatalog();

    //! 获取诊断报告(服务器生成报告)
    bool getReport(int slide_id, const QString &slide_name , const QString& conclusion);

    QString getUserName()  ;

    DeepLabelPrivate *data() { return this->_data; }
    QNetworkAccessManager * manager() {return this->_mgr; }



signals:
    void error(const QString& errorString);
    void finished();
    void sigProgress(int cur, int total, const QString& desc="");

private Q_SLOTS:
    void error(DeepLabelReply::Error error, const QString& errorString);
protected:
    QUrl url(const QString& path, const QVariantMap& params=QVariantMap());   
    DeepLabelReply* post(const QNetworkRequest &request, const QByteArray &data);

    DeepLabelReply* get(const QNetworkRequest &request);
    DeepLabelReply* get(QNetworkAccessManager* manager, const QNetworkRequest& request);
private:
    //  构造查询玻片的视野列表的请求
    QNetworkRequest _makeImageListRequest(int slide_id);

private:
    Q_DISABLE_COPY(DeepLabel)
    DeepLabelPrivate        *_data;
    QNetworkAccessManager   *_mgr;
};

#endif // DEEPLABEL_H
