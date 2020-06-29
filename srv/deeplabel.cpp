#include <QAuthenticator>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFuture>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QNetworkAccessManager>
#include <QtConcurrent>
#include <QUrlQuery>
#include "afxapp.h"
#include "commdefine.h"
#include "dbgutility.h"
#include "deeplabel.h"
#include "deeplabelreply.h"
#include "deepservice.h"
#include "slideitem.h"
#include "taskresult.h"
#include "categoryitem.h"
#include "commdefine.h"
#include <functional>

#pragma execution_character_set("utf-8")



class DeepLabelPrivate
{
public:
    DeepLabelPrivate():port(-1) // means absence
    {

    }
    QString host;
    int port;
    QString username;
    QString password;
    QString userType;
    QByteArray token;
    inline bool autherized() { return !token.isEmpty(); }
private:
    Q_DISABLE_COPY(DeepLabelPrivate)
};

DeepLabel::DeepLabel(QObject* parent)
    : QObject(parent)
    , _data(new DeepLabelPrivate)
    , _mgr(new QNetworkAccessManager(this))
{

}

DeepLabel::~DeepLabel()
{
    delete _data;
}

void DeepLabel::setHost(const QString& host, int port)
{
    data()->host = host;
    data()->port = port;
}

void DeepLabel::setCredential(const QString& username, const QString&password, const QString& userType)
{
    data()->username = username;
    data()->password = password;
    data()->userType = userType;
}

QString DeepLabel::login(bool* success)
{
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    request.setUrl(url("/api/login"));

    QUrlQuery params;
    params.addQueryItem("username", data()->username);
    params.addQueryItem("password", data()->password);
    params.addQueryItem("usertype", data()->userType);

    auto reply = post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    reply->waitForFinished();
    if (reply->error() != DeepLabelReply::NoError)
    {
        success && (*success = false);
        return reply->errorString();
    }
    success && (*success = true);
    auto json = reply->json();
    data()->token = json.value("token").toString().toUtf8();
    return QString();
}


/**
 * @brief 获取我的玻片列表. 调用接口/api/slides/my/slides/in/tasks实现, 返回玻片信息
 * @param start
 * @param count
 * @return
 * @note
 *      本函数不对玻片做条件过滤.
 */
DataList<SlideItem> DeepLabel::getTaskList(int start, int count)
{
    QString errString;
    bool isSuccess = false;
    // 如果没有login, 先login. 没有用处.
    if (!data()->autherized())
    {
        errString = login(&isSuccess);
        if( !isSuccess){
            return DataList<SlideItem>(0);
        }
    }

    // 查询
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/slides/my/slides/in/tasks", {
                        //{"assay_type_id", ""},
                        //{"device_id", ""},
                        //{"my_status", my_status==-1 ? "":QString::number(my_status)},
                        //{"slide_status", 1},
                          {"page", start / /*count*/MAX_PAGE_SIZE + 1},
                          {"size", count}}));

    TRACE() << "get my slide info. Request.url is: " << request.url();

    auto reply = get(request);
    reply->waitForFinished();
    if (reply->error() != DeepLabelReply::NoError)
    {
        errString = reply->errorString();
        ERROR() << "Failed to get my slides: " << errString;
        return DataList<SlideItem>(0);
    }

    auto json = reply->json();
    auto paging = json.value("page").toObject();
    DataList<SlideItem> dataList(paging.value("total_count").toInt());
    for (auto i : json.value("data").toArray())
    {
        SlideItem ti;
        auto item = i.toObject();
        ti.fromInTaskJson(item);
        dataList.append(ti);
    }

    return dataList;
}

DataList<SlideItem> DeepLabel::getUnassignedTaskList(int start, int count)
{
    QString errString;
    bool isSuccess = false;
    // 如果没有login, 先login. 没有用处.
    if (!data()->autherized())
    {
        errString = login(&isSuccess);
        if( !isSuccess){
            return DataList<SlideItem>(0);
        }
    }

    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/slides/take_list", {
                      // {"case_number", ""},
                      // {"assay_type_id", ""},
                      // {"device_id", ""},
                      {"page", start / count + 1},
                      {"size", count}}));
    TRACE() << "get unassigned slide, Request.url is: " << request.url();

    auto reply = get(request);
    reply->waitForFinished();

    if( reply->error() != DeepLabelReply::NoError)
    {
        errString = reply->errorString();
        ERROR() << "Fail to get unassigned slides.  " << errString;
        return DataList<SlideItem>(0);
    }

    auto json = reply->json();
    auto paging = json.value("page").toObject();
    DataList<SlideItem> dataList(paging.value("total_count").toInt());
    for (auto i : json.value("data").toArray())
    {
        SlideItem ti;
        ti.fromTakeListJson(i.toObject());
        dataList.append(ti);
    }
    return dataList;
}

void DeepLabel::takeTask(int uid)
{
    TRACE() << QString("Begin takeTask(%1)").arg(uid);
    if (data()->token.isEmpty())
    {
        qDebug() << "request for token" << endl;
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
        request.setUrl(url("/api/login"));
        qDebug() << request.url() << endl;

        QUrlQuery params;
        params.addQueryItem("username", data()->username);
        params.addQueryItem("password", data()->password);
        params.addQueryItem("usertype", data()->userType);

        auto reply = post(request, params.toString(QUrl::FullyEncoded).toUtf8());
        reply->waitForFinished();
        qDebug() << reply->error() << endl;
        if (reply->error() != DeepLabelReply::NoError)
        {
            return;
        }
        auto json = reply->json();
        data()->token = json.value("token").toString().toUtf8();
    }
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/tasks/take_all", {{"slide_id", uid}}));
    auto reply = get(request);
    reply->waitForFinished();
    TRACE() << QString("End takeTask(%1)").arg(uid);
}

void DeepLabel::untakeTask(int uid)
{
    TRACE() << QString("Begin untakeTask(%1)").arg(uid);
    if (data()->token.isEmpty())
    {
        qDebug() << "request for token" << endl;
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
        request.setUrl(url("/api/login"));
        qDebug() << request.url() << endl;

        QUrlQuery params;
        params.addQueryItem("username", data()->username);
        params.addQueryItem("password", data()->password);
        params.addQueryItem("usertype", data()->userType);

        auto reply = post(request, params.toString(QUrl::FullyEncoded).toUtf8());
        reply->waitForFinished();
        qDebug() << reply->error() << endl;
        if (reply->error() != DeepLabelReply::NoError)
        {
            return;
        }
        auto json = reply->json();
        data()->token = json.value("token").toString().toUtf8();
    }
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/tasks/untake_all", {{"slide_id", uid}, {"my_status", 1}}));
    auto reply = get(request);
    reply->waitForFinished();
    TRACE() << QString("End untakeTask(%1)").arg(uid);
}


/**
 * @brief 以
 * @param contour_id
 * @return
 */
QPolygon DeepLabel::getCellContours(const QString &contour_id)
{
    auto reply = fetchCellContours(contour_id);
    reply->waitForFinished();
    if( reply->error() != DeepLabelReply::NoError){
        ERROR() << "Fail to get contours " << contour_id << reply->errorString();
        return QPolygon();
    }
    auto rsp = reply->json();
    QPolygon contours;
    for(auto value: rsp.value("data").toArray())
    {
        auto point = value.toArray();
        contours.push_back(QPoint(point.at(0).toInt(), point.at(1).toInt()));
    }
    return contours;
}

DeepLabelReply *DeepLabel::fetchCellContours(const QString &contour_id)
{
    QNetworkRequest request;

    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/get_contours",
        {
            {"contours", contour_id}
        }));
    return get(request);
}

/**
 * @brief 获取一个玻片的所有视野, 细胞, 分类等信息. 涉及到多条命令.
 * @param slideId
 * @param   assayTypeId: 玻片类别.
 * @return
 * @note
 *  使用到的命令包括:
 *  - /api/tasks/mylist: 获取玻片下的所有视野信息
 *  - /api/actions/list: 针对一个视野, 获取它下面的所有的细胞的信息
 *  - ... 对一个视野, 获取它的图片
 */
TaskResult DeepLabel::getTaskResult(int slideId, int assayTypeId)
{

    auto t1 = GET_CLOCK();

    Q_ASSERT_X(assayTypeId!=TAssayType::BM_TYPE || assayTypeId!=TAssayType::PB_TYPE, "getTaskResult", "only support BM or PB type slide. ");
    TaskResult result(slideId, assayTypeId);
    // 登录
    QString errString;

    if( !data()->autherized()){
        login();
    }
    // 先获取分类字典.
    result.catalogs(this-> getCatalog());

    // 2. 查询玻片的视野列表
    QNetworkRequest request;

    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/tasks/mylist",
        {{"slide_id", slideId},
         {"image_name", ""},
         {"page", "1"},
         {"size", MAX_PAGE_SIZE}}));
    //TRACE() << "get image list. url is: " << request.url();
    //PRINT_CLOCK("begin query image list at ");
    INFO() << QString("Begin fetch image info. slide+id=%1").arg(slideId);
    auto reply = get(request);
    reply->waitForFinished();
    //PRINT_CLOCK("finished query image list result at ");

    auto rsp = reply->json();
    // 查询每个视野, 得到视野下的细胞信息
    for (auto item : rsp.value("data").toArray())
    {
        // 保存视野信息到TaskResult.images里面.
        ImageItem image;

        image.fromJson(item.toObject());
        // 暂时跳过10倍图. 不处理
        if( image.imageType() == ImageItem::TypeTile10x)
            continue;

        result.append(image);
        // 根据视野的类别来判断视野中找到的细胞的种类.
        // 百倍图中必然是白细胞(不考虑核型), 巨核中是巨核. 这个判断逻辑不一定正确.
        //QString cell_family = image.imageType()==ImageItem::TypeMega ? TCellFamily::FamilyMega : TCellFamily::FamilyHema;
        // 过滤出这个image使用的类别字典
        auto cell_catalog = filtFamily(result.catalogs(), TCellFamily::FamilyHema);

        // 查询每个视野中的细胞信息并下载细胞图像, 并保存到TaskResult.cells里面去.
        // /api/actions/list得到视野中所有的细胞的列表.
        QNetworkRequest request;
        request.setRawHeader("Authorization", data()->token);
        request.setUrl(url("/api/actions/list",
            {{"task_id", image.taskId()}
             ,{"image_id", image.id()}
             ,{"level", image.level()}
             ,{"mode", 0}       // 添加此参数, 不再下载细胞轮廓信息, 以节省时间.
            }));

        auto reply = get(request);
        reply->waitForFinished();
        auto rsp = reply->json();

        for (auto item: rsp.value("data").toObject())   // 每个item代表了一个细胞的信息.
        {
            auto obj = item.toObject();
            CellItem c;
            c.fromJson(obj);

            if (!c.isValid() || c.deleted())
            {
                ERROR() << QString("Invalid cell: ") << c.toString();
                continue;
            }

            //c.setFamily(cell_family);
            c.setCatalog(cell_catalog);
            c.setCategoryId(c.type());
            //TRACE() << "Parse cell: " << c.toString();
            // 下载细胞的图像文件.
            // 构造细胞图片的路径. 取决于数据库版本和细胞类别
            // 对于新的数据库, 细胞的图片路径在字段path中, 可以直接通过fullPath()得到全路径. 不需要再整合.
            // 对于老的数据库, 则需要通过slide_path等组合得到.
            QString cell_path;

            if( c.fullPath().isEmpty()) // fullPath没有数据的情况
            {
                c.setUrl(image.slidePath(), image.suffix());
            }
            else
            {
                cell_path = c.fullPath();
                c.setUrl(cell_path);
            }
            auto reply = download(c.url());

            reply->waitForFinished();
            if (reply->error() != DeepLabelReply::NoError)
            {
                ERROR() << "Fail to download cell image for " << c.uid() << ": " << reply->errorString();
                continue;
            }
            c.value()._pixmap.loadFromData(reply->rawData());

            result.append(c);
        }
    }
    auto t2 = GET_CLOCK();
    CLOCK() << "getTaskResult() consume " << t2-t1 << "msce";
    //PRINT_CLOCK("Leave getTaskResult ");

    return result;
}

/**
 * @brief 以并发方式获取玻片的数据信息, 并支持缓存方式.
 * @param slide 玻片信息SlideItem
 * @return  TaskResult的结构.
 * @note
 *      本函数使用多线程方式批量下载.
 */
TaskResult DeepLabel::getTaskResultEx(const SlideItem& slide)
{
    //QMap<int, CategoryItem> catalog;
    const QByteArray token = this->data()->token;
    int progress = 0;

    // -=`======= 线程函数定义
    // 定义下载视野的缩略图的线程
    std::function<void(ImageItem&)> fetchImagePreview = [this, token](ImageItem& image){
        QByteArray result;
        QByteArray data = downloadDataEx(image.thumbnail(), token);
        QPixmap pixmap;
        pixmap.loadFromData(data);
        image.setPrevPixmap(pixmap);
    };

    // 同步下载每个Cell的图像的线程.
    std::function<void(CellItem&)> fetchCellImage = [this, token](CellItem& cell){
        QByteArray data = downloadDataEx(cell.url(), token);
        if( !cell.isEmpyt() &&  !data.isEmpty()){
            cell.value()._pixmap.loadFromData(data);
        }
        if( cell.isEmpyt()){
            ERROR() << "Empty cell!" << cell.objectId();
        }
    };

    // 定义同步下载细胞信息. 如果支持批量下载getSlideCell(), 则不需要调用这个.
    typedef QList<CellItem> TCellList;
    // 下载一个Image下的所有cell
    std::function<TCellList(const ImageItem&)> fetchImageCell = [this, token](const ImageItem& image)->TCellList{
        TCellList result;
        QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
        QNetworkRequest request;
        request.setRawHeader("Authorization", token);
        request.setUrl(url("/api/actions/list",
            {{"task_id", image.taskId()}
             ,{"image_id", image.id()}
             ,{"level", image.level()}
             ,{"mode", 0}       // 添加此参数, 不再下载细胞轮廓信息, 以节省时间.
            }));
        auto reply = get(mgr, request);
        reply->waitForFinished();
        if( reply->error() != DeepLabelReply::NoError){
            ERROR() << "Fail to fetch cells for image" << image.id() << reply->errorString();
            return TCellList();
        }
        auto rsp = reply->json();
        for(auto item: rsp.value("data").toObject())
        {
            CellItem c;
            c.fromJson(item.toObject());

            if (!c.isValid() || c.deleted())
            {
                ERROR() << QString("Invalid cell: ") << c.toString();
                continue;
            }
            c.setSlideNo(image.slideNo());  //
            c.setCategoryId(c.type());
            //c.setFamily(cell_family);

            QString cell_path;
            if( c.fullPath().isEmpty()){
                c.setUrl(image.slidePath(), image.suffix());
            }
            else {
                c.setUrl(c.fullPath());
            }
            // 下载放在后面做.
            result.append(c);
        }
        mgr->deleteLater();
        return result;
    };

    // ============== 实际执行代码
    Q_ASSERT_X(slide.assayTypeId()!=TAssayType::BM_TYPE || slide.assayTypeId()!=TAssayType::PB_TYPE, "getTaskResult", "only support BM or PB type slide. ");
    TaskResult result(slide.slideId(), slide.assayTypeId());

    // 仅保存白细胞和巨核细胞的分类. 其他的都过滤掉.
    emit sigProgress(ProgressDefine::Init, ProgressDefine::Buddy, QObject::tr("下载分类定义..."));
    auto catalog = filtFamily(this->getCatalog(), TCellFamily::FamilyHema);
    result.catalogs(catalog);

    emit sigProgress(ProgressDefine::ReadCatalog, ProgressDefine::Buddy, QObject::tr("获取视野..."));
    // 获取视野列表.
    result.append( this->getImageListEx(slide) );
    emit sigProgress(ProgressDefine::ReadImage, ProgressDefine::Buddy, tr("获取细胞信息..."));

    // 构造一个slide_id到slide_no的字典.
    // 尝试使用批量下载的方式下载细胞数据.
    TCellList cells;
    cells = this->getSlideCells(slide, this->getUserName(), 0);
    if( cells.isEmpty()){
        QList<TCellList> cells_list = QtConcurrent::blockingMapped(result.imageList(), fetchImageCell);
        for(auto& item: cells_list){
            cells.append(item);
        }
    }
    result.append(cells);
    for(auto& item: result.cells()){
        item.setCatalog(catalog);
    }
    emit sigProgress(ProgressDefine::ReadCell, ProgressDefine::Buddy, tr("下载视野缩略图..."));

    // 下载每个视野的缩略图
    QtConcurrent::blockingMap(result.images(), fetchImagePreview);
    emit sigProgress(ProgressDefine::ReadImagePrev, ProgressDefine::Buddy, tr("下载细胞图像..."));
    // 以并行方式下载每个细胞的图像
    QtConcurrent::blockingMap(result.cells(), fetchCellImage);
    emit sigProgress(ProgressDefine::SetCatalog, ProgressDefine::Buddy, tr("设置分类视图..."));
    return result;
}



/**
 * @brief 用于异步方式下载的封装.
 * @param path
 * @return DeepLabelReply的指针.
 */
DeepLabelReply* DeepLabel::download(const QString &path)
{
    QNetworkRequest request;
    request.setUrl(url(path));
    return get(request);
}

DeepLabelReply *DeepLabel::download(QNetworkAccessManager *manager, const QString &path, const QByteArray& token)
{
    QNetworkRequest request;
    request.setRawHeader("Authorization", token);
    request.setUrl(url(path));
    return get(manager, request);
}




/**
 * @brief 以同步方式下载数据, 等待完成并返回数据内容.
 * @param path      网络服务器地址
 * @param isthrow   是否抛出异常.
 * @return
 */
QByteArray DeepLabel::downloadData(const QString &path, bool isthrow /*= false*/ )
{
    auto reply = this->download(path);
    reply->waitForFinished();
    if( reply->error() != DeepLabelReply::NoError){
        ERROR() << "Fail to download data from " << path << ": " << reply->errorString();
        if( isthrow){
            throw reply->errorString();
        }
    }
    return reply->rawData();
}


/**
 * @brief 同步方式支持缓存的方式下载数据. 如果缓存中有数据, 则直接使用缓存. 如果缓存没有, 下载后写入缓存.
 * @param path_name 资源的网络路径
 * @param isthrow   是否抛出异常, 当前暂不支持抛出异常
 * @return
 * @note
 *      DeepService::readCache()负责读缓存
 *      DeepService::writeCache()负责写缓存
 *      本函数内部调用download()实现get的下载功能, 要求path_name是网络资源的文件路径. 不支持查询等处理.
 */
QByteArray DeepLabel::downloadDataEx(const QString &path_name, const QByteArray& token)
{
    QByteArray result;

    QString file_path = DeepService::makeResourceCacheName(path_name);
    result = DeepService::loadRawData(file_path);
    if( !result.isEmpty())
    {
        return result;
    }
    //auto reply = this->download(path_name);
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    auto reply = this->download(mgr, path_name, token);

    reply->waitForFinished();
    if( reply->error() != DeepLabelReply::NoError)
    {
        ERROR() << "Fail to download data from " << path_name << reply->errorString();
        return QByteArray();
    }
    //
    result = reply->rawData();
    if( result.isEmpty())
    {
        ERROR() << "download empty data from " << path_name;
    }

    DeepService::cacheRawData(file_path, result);

    mgr->deleteLater();

    return result;


}

/**
 * @brief 同步查询玻片的视野列表, 不使用缓存
 * @param slide
 * @return
 */
QList<ImageItem> DeepLabel::getImageList(const SlideItem &slide)
{
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/tasks/mylist",
        {{"slide_id", slide.slideId()},
         {"image_name", ""},
         {"page", "1"},
         {"size", MAX_PAGE_SIZE}}));

    auto reply = get(request);
    reply->waitForFinished();

    if( reply->error() != DeepLabelReply::NoError){
        ERROR() << QString("Failed to get image list of slide %1. errorString is: %2").arg(slide.slideNo()).arg(reply->errorString());
        return QList<ImageItem>();
    }
    QList<ImageItem> result;
    auto rsp = reply->json();

    for(auto item: rsp.value("data").toArray())
    {
        ImageItem image;
        image.fromJson(item.toObject());
        if( image.imageType() == ImageItem::TypeTile10x){
            INFO() << "Skip 10x Image";
            continue;
        }
        result.append(image);
    }


    return result;
}

/**
 * @brief 获取玻片的视野列表. 如果有缓存则使用缓存.
 * @param slide 玻片信息
 * @return
 */
QList<ImageItem> DeepLabel::getImageListEx(const SlideItem &slide)
{
    QList<ImageItem> result;

    // 从缓存文件中加载. 如果缓存加载失败, rsp.isEmpty()返回true
    auto rsp = DeepService::loadJsonDoc(DeepService::makeImageListCacheName(slide.slideNo()) ).object();
    if( rsp.isEmpty())
    {
        QNetworkRequest request;
        request.setRawHeader("Authorization", data()->token);
        request.setUrl(url("/api/tasks/mylist",
            {{"slide_id", slide.slideId()},
             {"image_name", ""},
             {"page", "1"},
             {"size", MAX_PAGE_SIZE}}));

        auto reply = get(request);
        reply->waitForFinished();

        if( reply->error() != DeepLabelReply::NoError){
            ERROR() << QString("Failed to get image list of slide %1. errorString is: %2").arg(slide.slideNo()).arg(reply->errorString());
            return QList<ImageItem>();
        }
        //DeepService::cacheImageList(slide.slideNo(), reply->rawData());
        DeepService::cacheRawData(DeepService::makeImageListCacheName(slide.slideNo()), reply->rawData());
        //QList<ImageItem> result;
        rsp = reply->json();
    }
    // 从json中解析数据.
    for(auto item: rsp.value("data").toArray())
    {
        ImageItem image;
        image.fromJson(item.toObject());
        if( image.imageType() == ImageItem::TypeTile10x){
            INFO() << "Skip 10x Image";
            continue;
        }
        result.append(image);
    }
    return result;
}

/**
 * @brief 一次性下载玻片的所有细胞信息. 使用命令/api/actions/by/slide?mode=0&slide_id=5
 * @param slide     玻片信息. 命令中使用了slide_id字段
 * @param username  用户名. 当前用户名.
 * @param mode      0: 不包含轮廓; 1: 包含轮廓
 * @return
 * @note
 *  结果内容如下;
 *      - code
 *      - data  数组. 包含每个视野
 *          - 视野1
 *              - actions
 *                  - 细胞1
 *                      - 用户1: 细胞信息
 *                      - 用户2
 *                      - ...
 *                  - 细胞2
 *                  - ...
 *              - conclusion
 *              - id
 *              - images: 本细胞所在的视野的视野信息
 *              - level
 *          - 视野2
 *          - ...
 */
QList<CellItem> DeepLabel::getSlideCells(const SlideItem &slide, const QString &username, int mode)
{
    QList<CellItem> result;
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/actions/by/slide",
        {
            {"slide_id", slide.slideId()}
          , {"mode", mode}
        }));

    auto reply = get(request);
    reply->waitForFinished();
    if( reply->error() != DeepLabelReply::NoError){
        ERROR() << "Failed to get cells";
        return QList<CellItem>();
    }
    // data是一个array, 包含每个视野
    auto rsp = reply->json().value("data").toArray();
    // 处理每个视野.
    for(auto image: rsp)
    {
        // 即系并处理image的信息. 这里用得到
        auto imginfo = image.toObject().value("images").toObject();
        auto image_id = imginfo.value("id").toInt();
        auto image_type = imginfo.value("image_type").toInt();
        auto level   = image.toObject().value("level").toInt();
        //auto cell_family = image_type==ImageItem::TypeMega ? TCellFamily::FamilyMega : TCellFamily::FamilyHema;
        // 处理每个细胞
        auto cells = image.toObject().value("actions").toObject();
        for(auto item: cells)
        {
            CellItem c;
            c.fromJsonByName(item.toObject(), username, level);
            if( !c.isValid() || c.deleted()){
                ERROR() << QString("Invalid cell: ") << c.toString();
                continue;
            }
            c.setSlideNo(slide.slideNo());
            c.setCategoryId(c.type());
            //c.setFamily(cell_family);
            if( c.fullPath().isEmpty()){
                ERROR() << "New interface, path must not empty!";
            }
            else {
                c.setUrl(c.fullPath());
            }
            TRACE() << "image_id=" << image_id << " cells.id" << c.uid() << c.objectId() << c.imageId();
            result.append(c);
        }
    }

    TRACE() << "cell parsed. total " << result.size() << "cells";
    return result;
}



/**
 * @brief 保存结果. 只有有变更(含新增)的时候才会真正提交.
 * @param slide 玻片的信息. 使用里面的玻片id
 * @param cells 玻片中的细胞全集. 需要从中检查变更.
 * @return
 */
QList<CellItem> DeepLabel::saveResult(const SlideItem& slide, const QList<CellItem>& cells)
{
    TRACE();
    QList<CellItem> newCells;

    for (const CellItem& item : cells)
    {
        if (item.isValid())
        {
            if(item.deleted())
            {
                delCell(item.value()._id, item.value()._level);
            }
            else if (item.value()._category_id != item.value()._oldType)
            {
                modifyCellType(item.value()._id, item.value()._category_id, item.value()._level);
            }
        }
        else
        {
            if (!item.deleted())
            {
                CellItem c;
                c = addCell(slide.slideId(), item);
                // 注意, 这个细胞信息里面是没有字典信息的, 我们需要这个地方把他放进去.

                newCells.append(c);
            }
        }
    }

    return newCells;
}

bool DeepLabel::commitResult(int slide_id)
{
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", data()->token);


    request.setUrl(url("/api/images/commit",
        {{"id", slide_id}}));

    TRACE() << "Commit: " << request.url();
#if 1
    auto reply = get(request);

    reply->waitForFinished();
    if( reply->error() != reply->NoError){
        ERROR() << "Commit Failed!" << reply->errorString();
        return false;
    }
#endif
    return true;
}

/**
 * @brief 删除一个细胞
 * @param cell_id   细胞的uid, 来自CellItem._value.id
 * @param level     当前的level, 来自Cellitem._value.level
 * @note
 *      失败了只做一下记录, 不做其他的处理.
 */
void DeepLabel::delCell(int cellId, int level)
{
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/actions/delete",
        {{"id", cellId},
         {"level", level}}));
    auto reply = get(request);
    reply->waitForFinished();
    if( reply->error() != DeepLabelReply::NoError){
        ERROR() << "Fail to delete cell: cell_id=" << cellId << " level = " << level;
    }
}

/**
 * @brief 修改已有的细胞的类型.
 * @param cellId
 * @param cellType
 * @param level
 */
void DeepLabel::modifyCellType(int cellId, int cellType, int level)
{
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/actions/edit/type",
        {{"id", cellId},
         {"object_type_id", cellType},
         {"level", level}}));
    auto reply = get(request);
    reply->waitForFinished();
    if( reply->error() != DeepLabelReply::NoError){
        ERROR() << QString("Fail to modify cell's type: cellId=%1, cellType=%2, level=%3")
                   .arg(cellId).arg(cellType).arg(level);
    }
}

/**
 * @brief 新增一个细胞的信息.
 * @param data
 */
CellItem DeepLabel::addCell(int slideId, const CellItem &cell)
{
    TRACE();
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/actions/add"));
    QJsonArray contour;
    std::transform(cell.value()._contours.begin(), cell.value()._contours.end(), std::back_inserter(contour), [&](const QPoint& p) {
        return QJsonArray({p.x(), p.y()});
    });
    auto rect = cell.value()._contours.boundingRect();
    QJsonObject data;
    data["task_id"]     = cell.taskId();
    data["slide_id"]    = slideId;
    data["image_id"]    = cell.imageId();
    data["action"]      = 1;        // 1表示新增
    data["object_id"]   = "new";
    data["contours"]    = contour;
    data["rotation"]    = 0;
    data["level"]       = cell.level();
    data["object_type_id"] = cell.type();
    data["outlines"] = QString(QJsonDocument({{"x", rect.x()}, {"y", rect.y()}, {"width", rect.width()}, {"height", rect.height()}}).toJson(QJsonDocument::Compact));

    auto reply = post(request, QJsonDocument(data).toJson(QJsonDocument::Compact));
    reply->waitForFinished();
    if( reply->error() != DeepLabelReply::NoError)
    {
        ERROR() << QString("Failed to save cell") << reply->errorString();
    }
    auto rsp = reply->json();

    auto obj = rsp.value("data").toObject();
    CellItem c;
    c.fromJson(QJsonObject({{"new", obj}}));
    if (c.isValid())
    {
        c.value()._pixmap = cell.pixmap();
        //c.setFamily(cell.getFamily());
        c.setCatalog(cell.getCatalog());
        c.setCategoryId(cell.getCategoryId());

    }
    return c;
}



QMap<int, QString> DeepLabel::getTypes()
{
    QMap<int, QString> mapping;
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/object_types/all"));
    auto reply = get(request);
    reply->waitForFinished();
    if (reply->error() == reply->NoError)
    {
        auto rsp = reply->json();
        for (auto item: rsp.value("data").toArray())
        {
            auto obj = item.toObject();
            mapping.insert(obj.value("id").toInt(), obj.value("category").toString());
        }
    }

    return mapping;
}
/**
 * @brief 获取细胞的分类名称定义
 * @return
 * @note
 *      id: 全局类型.
 *      category: 中文名称
 *      category_en: 英文名称
 *      category_no: 名称简写(形态来说, 和英文名称一样)
 */
QMap<int, CategoryItem> DeepLabel:: getCatalog()
{
    QMap<int, CategoryItem> mapping;
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/object_types/all"));
    auto reply = get(request);
    reply->waitForFinished();
    if (reply->error() == reply->NoError)
    {
        auto rsp = reply->json();
        for (auto item: rsp.value("data").toArray())
        {
            auto obj = item.toObject();
            CategoryItem typeItem;
            typeItem.fromJson(obj);
            mapping.insert(typeItem.id, typeItem);
        }
    }

    return mapping;
}

/**
 * @brief 获取诊断报告(PDF格式)
 * @param conclusion 诊断结论,
 * @return
 */
bool DeepLabel::getReport(int slide_id, const QString& slide_name, const QString &conclusion)
{
    if( !data()->autherized())
    {
        login();
    }
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/slides/pdf/browse"));

    QJsonObject data;
    data["slide_id"] = slide_id;
    data["conclusion"] = conclusion;
    auto reply = post(request, QJsonDocument(data).toJson(QJsonDocument::Compact));

    reply->waitForFinished();
    if( reply->error() != reply->NoError){
        TRACE() << "Error " << reply->errorString();
        return false;
    }
    auto rsp = reply->json();
    TRACE() << rsp.value("path").toString();

    auto reply2=this->download(rsp.value("path").toString());
    reply2->waitForFinished();
    if( reply2->error() != reply2->NoError)
    {
        TRACE() << "Error " << reply2->errorString();
        return false;
    }

    QFile file(DeepService::getReportFilePath(this->data()->username,slide_name));
    file.open(QFile::WriteOnly);
    file.write(reply2->rawData());
    file.close();

    return true;
}

QString DeepLabel::getUserName()
{
    return this->data()->username;
}




QUrl DeepLabel::url(const QString& path, const QVariantMap& params)
{
    QUrl url;
    url.setScheme("http");
    url.setHost(data()->host);
    url.setPort(data()->port);
    url.setPath(path);
    QUrlQuery query(url);
    for (auto iter = params.begin(); iter != params.end(); ++iter)
    {
        query.addQueryItem(iter.key(), iter->toString());
    }
    url.setQuery(query);
    return url;
}

DeepLabelReply* DeepLabel::post(const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkReply* reply = manager()->post(request, data);
    auto dlReply = new DeepLabelReply(reply, this);
    connect(dlReply, QOverload<DeepLabelReply::Error, const QString&>::of(&DeepLabelReply::error),
            this, QOverload<DeepLabelReply::Error, const QString&>::of(&DeepLabel::error));
    return dlReply;
}



DeepLabelReply* DeepLabel::get(const QNetworkRequest &request)
{
    QNetworkReply* reply = manager()->get(request);
    auto dlReply = new DeepLabelReply(reply, this);
    connect(dlReply, QOverload<DeepLabelReply::Error, const QString&>::of(&DeepLabelReply::error),
            this, QOverload<DeepLabelReply::Error, const QString&>::of(&DeepLabel::error));
    return dlReply;
}

DeepLabelReply *DeepLabel::get(QNetworkAccessManager *manager, const QNetworkRequest &request)
{
    QNetworkReply* reply = manager->get(request);
    auto dlReply = new DeepLabelReply(reply, this);
    connect(dlReply, QOverload<DeepLabelReply::Error, const QString&>::of(&DeepLabelReply::error),
            this, QOverload<DeepLabelReply::Error, const QString&>::of(&DeepLabel::error));
    return dlReply;
}



//! 这个函数暂时不使用了.
QNetworkRequest DeepLabel::_makeImageListRequest(int slide_id)
{
    QNetworkRequest request;
    request.setRawHeader("Authorization", data()->token);
    request.setUrl(url("/api/tasks/mylist",
        {{"slide_id", slide_id},
         {"image_name", ""},
         {"page", "1"},
         {"size", "100"}}));

    return request;
}

void DeepLabel::error(DeepLabelReply::Error error, const QString& errorString)
{
    Q_UNUSED(error)
    emit this->error(errorString);
}
