#ifndef DEEPLABELREPLY_H
#define DEEPLABELREPLY_H

#include <QObject>
#include <QNetworkReply>
#include <QMetaType>
class DeepLabelReplyPrivate;
class DeepLabelReply : public QObject
{
    Q_OBJECT
public:
    enum Error
    {
        NoError,
        CommunicationError,
        ParseError,
        ApplicationError,
        UnknownError,
    };

    explicit DeepLabelReply(QNetworkReply *reply, QObject *parent = nullptr);
    ~DeepLabelReply();
    Error error() const;
    QString errorString() const;
    bool isFinished() const;
    QByteArray rawData() const;
    QJsonObject json() const;
    void waitForFinished();
protected:
    void setError(Error error, const QString &errorString);
    void setFinished(bool finished);
    void setRawData(const QByteArray& ba);
    void setJson(const QJsonObject& json);

signals:
    void finished();
    void error(Error error, const QString& errorString=QString());

private Q_SLOTS:
    void networkFinished();
    void networkError(QNetworkReply::NetworkError);

private:
    Q_DISABLE_COPY(DeepLabelReply)
    DeepLabelReplyPrivate *data;
};

Q_DECLARE_METATYPE(DeepLabelReply::Error)
#endif // DEEPLABELREPLY_H
