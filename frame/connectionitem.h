#ifndef CONNECTIONITEM_H
#define CONNECTIONITEM_H

#include <QMetaType>
#include <QString>

class ConnectionItem
{
public:
    QString profileName;
    QString domainName;
    int port;
    QString username;
    QString password;
    bool remember;
};

Q_DECLARE_METATYPE(ConnectionItem)
#endif // CONNECTIONITEM_H
