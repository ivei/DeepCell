#include "deeplabelreply.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QEventLoop>
#include "dbgutility.h"

#pragma execution_character_set("utf-8")

class DeepLabelReplyPrivate
{
public:
    DeepLabelReplyPrivate():error(DeepLabelReply::NoError), isFinished(false){}
    DeepLabelReply::Error error;
    QString errorString;
    bool isFinished;
    QByteArray rawData;
    QJsonObject json;
private:
    Q_DISABLE_COPY(DeepLabelReplyPrivate)
};

DeepLabelReply::DeepLabelReply(QNetworkReply *reply, QObject *parent) : QObject(parent), data(new DeepLabelReplyPrivate)
{
    Q_ASSERT(reply);
    qRegisterMetaType<Error>("Error");
    connect(reply, &QNetworkReply::finished, this, &DeepLabelReply::networkFinished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &DeepLabelReply::networkError);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

DeepLabelReply::~DeepLabelReply()
{
    delete data;
}

DeepLabelReply::Error DeepLabelReply::error() const
{
    return data->error;
}

QString DeepLabelReply::errorString() const
{
    return data->errorString;
}

bool DeepLabelReply::isFinished() const
{
    return data->isFinished;
}

QByteArray DeepLabelReply::rawData() const
{
    return data->rawData;
}

QJsonObject DeepLabelReply::json() const
{
    return data->json;
}

void DeepLabelReply::setError(Error error, const QString &errorString)
{
    data->error = error;
    data->errorString = errorString;
    emit this->error(error, errorString);
    setFinished(true);
}

void DeepLabelReply::setFinished(bool isFinished)
{
    data->isFinished = isFinished;
    if (isFinished)
    {
        emit this->finished();
    }
}

void DeepLabelReply::setRawData(const QByteArray &ba)
{
    data->rawData = ba;
}

void DeepLabelReply::setJson(const QJsonObject &json)
{
    data->json = json;
}

void DeepLabelReply::waitForFinished()
{
    QEventLoop loop;
    connect(this, &DeepLabelReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

void DeepLabelReply::networkFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        return;
    }

    auto rawData = reply->readAll();
    setRawData(rawData);

    auto contentType = reply->header(QNetworkRequest::ContentTypeHeader);
    if (contentType.isValid() && contentType == "application/json")
    {
        QJsonParseError parseError;
        auto doc = QJsonDocument::fromJson(rawData, &parseError);
        if (parseError.error != QJsonParseError::NoError)
        {
            setError(ParseError, parseError.errorString());
            return;
        }
        QJsonObject obj;
        if (!doc.isObject() || !(obj = doc.object()).contains("code"))
        {
            setError(ParseError, "Invalid response.");
            return;
        }
        if (obj.value("code") != 200)
        {
            setError(ApplicationError, obj.value("message").toString());
            return;
        }
        setJson(obj);
    }

    setFinished(true);
}

void DeepLabelReply::networkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    setError(CommunicationError, reply->errorString());
    reply->deleteLater();
}




