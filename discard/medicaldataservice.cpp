#include "connectionprofile.h"
#include "medicaldataservice.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QUrlQuery>
#include <QImageReader>
#include <QImage>
#include <QVariantMap>
#include <QException>
#include <QEventLoop>
#include <QScopedPointer>
#include <QtDebug>
#include "../ui/dbgutility.h"

QMap<QString, int> mapping = {
    {"NO_0", 0},
    {"NO_1", 1},
    {"NO_2", 2},
    {"NO_3", 3},
    {"NO_4", 4},
    {"NO_5", 5},
    {"NO_6", 6},
    {"NO_7", 7},
    {"NO_8", 8},
    {"NO_9", 9},
    {"NO_10", 10},
    {"NO_11", 11},
    {"NO_12", 12},
    {"NO_13", 13},
    {"NO_14", 14},
    {"NO_15", 15},
    {"NO_16", 16},
    {"NO_17", 17},
    {"NO_18", 18},
    {"NO_19", 19},
    {"NO_20", 20},
    {"NO_21", 21},
    {"NO_22", 22},
    {"NO_X", 23},
    {"NO_Y", 24},
};

QMap<int, QString> remapping = {
    {0, "NO_0"},
    {1, "NO_1"},
    {2, "NO_2"},
    {3, "NO_3"},
    {4, "NO_4"},
    {5, "NO_5"},
    {6, "NO_6"},
    {7, "NO_7"},
    {8, "NO_8"},
    {9, "NO_9"},
    {10, "NO_10"},
    {11, "NO_11"},
    {12, "NO_12"},
    {13, "NO_13"},
    {14, "NO_14"},
    {15, "NO_15"},
    {16, "NO_16"},
    {17, "NO_17"},
    {18, "NO_18"},
    {19, "NO_19"},
    {20, "NO_20"},
    {21, "NO_21"},
    {22, "NO_22"},
    {23, "NO_X"},
    {24, "NO_Y"},
};

MedicalDataService::MedicalDataService(QObject *parent) : QObject(parent), profile(new ConnectionProfile), netMgr(new QNetworkAccessManager(this))
{

}

MedicalDataService::MedicalDataService(const ConnectionProfile& profile, QObject *parent):QObject(parent),
    profile(new ConnectionProfile(profile)), netMgr(new QNetworkAccessManager(this))
{


}

MedicalDataService::MedicalDataService(const MedicalDataService& other): profile(other.profile), netMgr(other.netMgr),
    token(other.token), status(other.status)
{

}

void MedicalDataService::setConnectionProfile(const ConnectionProfile& profile)
{
    *this->profile = profile;

}

void MedicalDataService::requestToken()
{

    status = NOT_AUTHORIZED;
    token.clear();
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    buildRequest(request, "api/login");

    QUrlQuery params;
    params.addQueryItem("username", profile->getUsername());
    params.addQueryItem("password", profile->getPassword());
    params.addQueryItem("usertype", "user");
//    connect(&netMgr, &QNetworkAccessManager::finished, [](QNetworkReply *reply){
//        qDebug() << reply->readAll() << endl;
//    });

    QNetworkReply* reply = netMgr->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &MedicalDataService::processToken);

}

/**
 * @brief 查询用户名下的任务列表信息. 返回的是用户名下所有task所从属的slide的列表
 * @param start
 * @param count
 * @details
 *      这个函数用于当用户点击数据库连接后执行, 查询用户名下的slide信息.
 *      实际上查询的是task信息, 后台将task按照slide分组后, 返回slide的信息.
 *      真正需要关注的信息, 参考SpecimanTableModel::data()函数
 */
void MedicalDataService::fetchAssayList(int start, int count)
{
    QNetworkRequest request;
    buildRequest(request, "api/slides/my/slides/in/tasks", {{"page", QString::number(start / count + 1)}, {"size", QString::number(count)}});
    QNetworkReply* reply = netMgr->get(request);
    connect(reply, &QNetworkReply::finished, this, &MedicalDataService::processJsonReply);
}

void MedicalDataService::fetchAssayResultList(const QString& assayId)
{
    TRACE() ;
    QNetworkRequest request;
    buildRequest(request, "api/tasks/mylist", {{"slide_id", assayId}, {"image_name", ""}, {"page", "1"}, {"size", "100"}});
    TRACE() << "the request url is: " << request.url();
    QNetworkReply* reply = netMgr->get(request);
    connect(reply, &QNetworkReply::finished, this, &MedicalDataService::processJsonReply);
}

void MedicalDataService::fetchTodoTaskList(int start, int count)
{
    QNetworkRequest request;
    buildRequest(request, "/api/slides/take_list", {{"page", QString::number(start / count + 1)}, {"size", QString::number(count)}});
    QNetworkReply* reply = netMgr->get(request);
    connect(reply, &QNetworkReply::finished, this, &MedicalDataService::processJsonReply);
}

void MedicalDataService::fetchTodoTaskDetails(const QString& taskId)
{
    QNetworkRequest request;
    buildRequest(request, "/api/images/my/images/on/take", {{"slide_id", taskId}, {"image_name", ""}, {"page", "1"}, {"size", "100"}});
    QNetworkReply* reply = netMgr->get(request);
    connect(reply, &QNetworkReply::finished, this, &MedicalDataService::processJsonReply);
}

void MedicalDataService::takeTodoTask(const QString& slideNo, const QStringList& items)
{
    Q_UNUSED(slideNo)
    QNetworkRequest request;
    buildRequest(request, "api/tasks/take", {{"images", items.join(',')}});
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString() << endl;
    }

    try
    {
         parseResult(reply->readAll());
         return;
    }
    catch(const QString& errString)
    {
         qDebug() << errString << endl;
         return;
    }
}

void MedicalDataService::untakeTodoTask(const QString& slideNo, const QStringList& items)
{
    Q_UNUSED(slideNo)
    QNetworkRequest request;
    buildRequest(request, "api/tasks/untake", {{"images", items.join(',')}});
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString() << endl;
    }

    try
    {
         parseResult(reply->readAll());
         return;
    }
    catch(const QString& errString)
    {
         qDebug() << errString << endl;
         return;
    }
}

std::vector<Chromosome> MedicalDataService::fetchAssayResultDetails(
        const QString& taskId,
        const QString& imageId,
        const QString& level) const
{
	TRACE();
    int lvl = level.toInt() - 1;

    std::vector<Chromosome> objects[3];

    QNetworkRequest request;
    buildRequest(request, "api/actions/list", {{"task_id", taskId}, {"image_id", imageId}, {"level", level}});

    TRACE() << "request url is: " << request.url();

    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString() << endl;

        return objects[lvl];
    }

    try
    {
        auto rsp = parseResult(reply->readAll()).toVariantMap()["data"].toMap();
        TRACE() << "rsp.size() : " << rsp.size() << endl;
        auto categories = fetchTypeMaping();
        for (auto iter = rsp.begin(); iter != rsp.end(); ++iter)
        {
            auto acts = iter.value().toMap();
            auto lastest = std::max_element(acts.begin(), acts.end(), [](const QVariant& l, const QVariant& r)
            {
                return l.toMap()["id"].toInt() < r.toMap()["id"].toInt();
            })->toMap();

            Chromosome obj;
            obj.id = iter.key().toStdString();
            obj.type = mapping[categories.value(lastest["object_type_id"].toInt())];

            obj.angleRotate = lastest["rotation"].toInt();

            Q_FOREACH(auto const &value, lastest["contours"].toList())
            {
                auto point = value.toList();
                obj.contour.push_back(cv::Point(point[0].toInt(), point[1].toInt()));
            };

            objects[lastest["level"].toInt() - 1].push_back(obj);
        }
    }
    catch(const QString& errString)
    {
         qDebug() << errString << endl;
    }
    if (lvl > 0 && objects[lvl].empty())
    {
        --lvl;
    }

    return objects[lvl];
}

// 从数据库中查询这张图片的处理结果
bool MedicalDataService::fetchAssayResultStatus(
        const QString& taskId,
        const QString& imageId,
        const QString& level
    ) const
{

    QNetworkRequest request;
    buildRequest(request, "api/actions/list", {{"task_id", taskId}, {"image_id", imageId}, {"level", level}});
    TRACE() << "url: " << request.url();
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString() << endl;
        return false;
    }

    try
    {
        return !parseResult(reply->readAll()).take("data").toObject().empty();
    }
    catch(const QString& errString)
    {
         qDebug() << errString << endl;
         return false;
    }
}

std::vector<cv::Point> MedicalDataService::fetchObjectControus(const QString& objectId) const
{
    QNetworkRequest request;
    buildRequest(request, "api/get_contours", {{"contours", objectId}});
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError)
    {
        throw reply->errorString();
    }
    std::vector<cv::Point> contours;
    Q_FOREACH(const QJsonValue& obj, parseResult(reply->readAll())["data"].toArray())
    {
        auto point = obj.toArray();
        contours.push_back(cv::Point(point[0].toInt(), point[1].toInt()));
    };
    return contours;
}

QMap<int, QString> MedicalDataService::fetchTypeMaping() const
{
    QNetworkRequest request;
    buildRequest(request, "api/object_types/all");
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError)
    {
        throw reply->errorString();
    }
    QMap<int, QString> categories;
    Q_FOREACH(const QJsonValue& obj, parseResult(reply->readAll())["data"].toArray())
    {
        auto point = obj.toObject();
        categories.insert(obj["id"].toInt(), obj["object_type_no"].toString());
    };
    return categories;
}


// 保存核型结果时, 上传核型的数据.
void MedicalDataService::putAssayResult(
        const QString& taskId,
        const QString& imageId,
        const QString& level,           //! 2019-12-04 新增参数
        QVector<Chromosome>& result)
{

    QNetworkRequest request;
    buildRequest(request, "/api/actions/delete/by/task", {{"task_id", taskId}});
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString() << endl;;
    }

    auto categories = fetchTypeMaping();
    QMap<QString, int> recat;
    for (auto iter = categories.begin(); iter != categories.end(); ++iter)
    {
        recat.insert(*iter, iter.key());
    }
    {
        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        buildRequest(request, "api/actions/multi/add");

        QJsonArray objects;
        for (auto iter = result.begin(); iter != result.end(); ++iter)
        {
            QJsonArray contour;
            std::transform(iter->contour.begin(), iter->contour.end(), std::back_inserter(contour), [&](const cv::Point& p) {
                return QJsonArray({p.x, p.y});
            });
            auto rect = cv::boundingRect(iter->contour);
            QJsonObject data;
            data["task_id"] = taskId;
            data["image_id"] = imageId;
            data["action"] = 1;
            data["contours"] = contour;
            data["rotation"] = static_cast<int>(iter->angleRotate);
            //!data["level"] = "1";
            data["level"] = level;
//            data["object_id"] = "new";
            //TRACE() << "SaveToJson: " << iter->type;
            data["object_type_id"] = recat[remapping[iter->type]];
            data["outlines"] = QString(QJsonDocument({{"x", rect.x}, {"y", rect.y}, {"width", rect.width}, {"height", rect.height}}).toJson(QJsonDocument::Compact));
            objects.append(data);
        }

        QJsonObject body;
        body["image_id"] = imageId;
        body["data"] = objects;
        QScopedPointer<QNetworkReply> reply(netMgr->post(request, QJsonDocument(body).toJson(QJsonDocument::Compact)));
        QEventLoop loop;
        connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << reply->errorString() << endl;

            return;
        }
        try
        {
            parseResult(reply->readAll());
        }
        catch(const QString& errString)
        {
            qDebug() << errString << endl;
        }
    }
}

void MedicalDataService::saveAssayDetail(const QString& imageId, const QVariantMap& details)
{
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    buildRequest(request, "/api/tasks/save");

    QJsonObject data;
    data["image_id"] = imageId;
    for (auto iter = details.begin(); iter != details.end(); ++iter)
    {
        data[iter.key()] = QJsonValue::fromVariant(*iter);
    }
    QScopedPointer<QNetworkReply> reply(netMgr->post(request, QJsonDocument(data).toJson(QJsonDocument::Compact)));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString() << endl;
    }
}


bool MedicalDataService::commitResult(const QString& imageId, const QVariantMap& details)
{
    QNetworkRequest request;
    buildRequest(request, "/api/tasks/commit");
    QJsonObject data;
    data["image_id"] = imageId;

	for (auto iter = details.begin(); iter != details.end(); ++iter)
    {
        data[iter.key()] = QJsonValue::fromVariant(*iter);
    }
    QScopedPointer<QNetworkReply> reply(netMgr->post(request, QJsonDocument(data).toJson(QJsonDocument::Compact)));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();

    if( reply->error() != QNetworkReply::NoError)
    {
        TRACE() << "Commit failed! : " << reply->errorString();
        return false;
    }

    return true;

}

// 下载缩略图使用. 
QImage MedicalDataService::downloadImage(const QString& path)
{
    QNetworkRequest request;
    buildRequest(request, path);
    TRACE() << request.url();
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() == QNetworkReply::NoError)
    {
        QImageReader imageReader(reply.data());
        return imageReader.read();
    }
    else
    {
        TRACE() << "download preview error: " << reply->errorString() << "url is: " << request.url();
        return QImage();
    }
}

bool MedicalDataService::downloadImage(const QString& path, QImage& image)
{
    QNetworkRequest request;
    buildRequest(request, path);
    TRACE() << request.url();
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() == QNetworkReply::NoError)
    {
        QImageReader imageReader(reply.data());
        TRACE() << "readImage file from network: " << imageReader.errorString();
        return imageReader.read(&image);
    }
    else
    {
        TRACE() << "download preview error: " << reply->errorString() << "url is: " << request.url();
        return false;
    }
}

QNetworkReply* MedicalDataService::get(const QString& path)
{
    QNetworkRequest request;
    buildRequest(request, path);
    return netMgr->get(request);

}

void MedicalDataService::processToken()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError)
    {
        try
        {
            auto rsp = parseResult(reply->readAll());
            token = rsp["token"].toString();
            status = GRANTED;
            emit authorized();
        }
        catch(const QString& err)
        {
            emit error(err);
        }
    }
    else
    {
        emit error(reply->errorString());
    }
    reply->deleteLater();

}

void MedicalDataService::processJsonReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError)
    {
        try
        {
            emit done(parseResult(reply->readAll()));
        }
        catch(const QString& err)
        {
            emit error(err);
        }
    }
    else
    {
        emit error(reply->errorString());
    }

    reply->deleteLater();

}

#if 0
// 用于处理异步下载图像后的处理. 
// 当前只用于下载样片的缩略图. 大图的下载在fetchFullImage()中实现. 
void MedicalDataService::processImageDownload(QNetworkReply* reply, const QString& path)
{
	TRACE() << "MedicalDataService::processImageDownload" << "	reply.error() is: " << reply->error() << "	path is: "  << path;
    if (reply->error() == QNetworkReply::NoError)
    {
        QImageReader imageReader(reply);
        QImage image = imageReader.read();
        if (!image.isNull())
        {
            emit imageDone(path, image);

        }
        else
        {
            emit error("Invalid image format.");
        }
    }
    else
    {
        emit error(reply->errorString());
    }

    reply->deleteLater();

}

#endif

bool MedicalDataService::fetchFullImage(const QString& path, QImage& img)
{
	TRACE() << "path : " << path;
    QNetworkRequest request;
    buildRequest(request, path);
    QScopedPointer<QNetworkReply> reply(netMgr->get(request));
    QEventLoop loop;
    connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() != QNetworkReply::NoError)
    {
        TRACE() << reply->errorString();
        return false;
    }
	TRACE() << "read finished";
    QImageReader imageReader(reply.data());
	img = imageReader.read();
	TRACE() << "read image, size is: " << img.size();

    return !img.isNull() ;
}

void MedicalDataService::buildRequest(QNetworkRequest& req, const QString& query, const QueryItemList& queryItems) const
{
    QUrl url(QString("%1/%2").arg(profile->getUrl().toString()).arg(query));
	
    if (!url.isValid())
    {
        return;
    }

    QUrlQuery urlQuery;
    urlQuery.setQueryItems(queryItems);
    url.setQuery(urlQuery);

    req.setUrl(url);
//    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (GRANTED == status)
    {
        req.setRawHeader("Authorization", token.toUtf8());
    }

}

QJsonObject MedicalDataService::parseResult(const QByteArray& data) const
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error)
    {
        throw parseError.errorString();
    }

    if (!doc.isObject())
    {
        throw QString("Wrong result format");
    }

    QJsonObject rsp = doc.object();

    switch (rsp["code"].toInt(0)) {
    case 200:
        return rsp;
    default:
        throw rsp["message"].toString();
    }

}
