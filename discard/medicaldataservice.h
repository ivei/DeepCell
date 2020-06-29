#ifndef MEDICALDATASERVICE_H
#define MEDICALDATASERVICE_H
#include <QObject>
#include <QSharedPointer>
#include <QList>
#include <QPair>
#include "imageutil.h"
#include "dcutility.h"




class QJsonDocument;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class ConnectionProfile;
class MedicalDataService: public QObject
{
    Q_OBJECT
public:
    enum AuthStatus{
        NOT_AUTHORIZED = 0,
        GRANTED,
        EXPIRED,
    };
    explicit MedicalDataService(QObject *parent = nullptr);
    explicit MedicalDataService(const MedicalDataService& other);

    MedicalDataService(const ConnectionProfile& profile, QObject *parent = nullptr);
    void setConnectionProfile(const ConnectionProfile& profile);

    void requestToken();
    void fetchAssayList(int start, int count);
    void fetchAssayResultList(const QString& taskId);
    void fetchTodoTaskList(int start, int count);
    void fetchTodoTaskDetails(const QString& slideNo);
    void takeTodoTask(const QString& slideNo, const QStringList& items);
    void untakeTodoTask(const QString& slideNo, const QStringList& items);

    //!2019-12-04 增加level参数
    std::vector<Chromosome> fetchAssayResultDetails(
            const QString& taskId,
            const QString& imageId,
            const QString& level) const;
    bool fetchAssayResultStatus(const QString& taskId, const QString& imageId, const QString& level) const;
    //! 2019-12-04 END
    void putAssayResult(const QString& taskId, const QString& imageId, const QString& level,QVector<Chromosome>& result);
    void saveAssayDetail(const QString& imageId, const QVariantMap& details);
    bool commitResult(const QString& imageId, const QVariantMap& details);
    QImage downloadImage(const QString& path);
    bool downloadImage(const QString& path, QImage& image);
    bool fetchFullImage(const QString& path, QImage& img);
    QNetworkReply* get(const QString& path);

    static const QString getViewPreviewPicPath(const QString& path, const QString& name)
    {
        return QString("%1/preview/%2").arg(path).arg(name);
    }

    static const QString getViewFullPicPath(const QString& path, const QString& name)
    {
        return QString("%1/data/%2").arg(path).arg(name);
    }

signals:
    void authorized();
    void error(const QString& errorString);
    void done(const QJsonObject&);
    void imageDone(const QString&, const QImage&);
    void jsonDone(const QJsonDocument& doc);

protected slots:
    void processJsonReply();
    //void processImageDownload(QNetworkReply*, const QString&);
    void processToken();
    std::vector<cv::Point> fetchObjectControus(const QString& objectId) const;
    QMap<int, QString> fetchTypeMaping() const;

private:
    typedef QList<QPair<QString, QString>> QueryItemList;

    void buildRequest(QNetworkRequest& req, const QString& query, const QueryItemList& queryItems=QueryItemList()) const;

    QJsonObject parseResult(const QByteArray&) const;

    QSharedPointer<ConnectionProfile> profile;
    QSharedPointer<QNetworkAccessManager> netMgr;
    QString token;
    AuthStatus status;

};


Q_DECLARE_METATYPE(QVector<Chromosome>)

#endif // MEDICALDATASERVICE_H
