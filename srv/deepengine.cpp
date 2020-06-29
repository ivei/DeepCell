#include <QFile>
#include "deepengine.h"
#include "deeplabel.h"
#include <QCoreApplication>
#include <QDataStream>

#pragma execution_character_set("utf-8")

QDataStream & operator<<(QDataStream &out, const ConnectionItem& conn)
{
    return out << conn.profileName
        << conn.domainName
        << conn.port
        << conn.username
        << (conn.remember ? conn.password : "")
        << conn.remember;
}

QDataStream & operator>>(QDataStream &in, ConnectionItem& conn)
{
    return in >> conn.profileName
              >> conn.domainName
              >> conn.port
              >> conn.username
              >> conn.password
              >> conn.remember;
}

DeepEngine::DeepEngine() : QObject(nullptr)
{

}

QList<ConnectionItem> DeepEngine::getConnections()
{
    QList<ConnectionItem> connections;
    QFile file(QCoreApplication::applicationDirPath() + "/connect.bin");

    if (!file.open(QIODevice::ReadWrite))
    {
        return connections;
    }
    QByteArray data = file.readAll();
    file.close();
    QDataStream ds(&data, QFile::ReadOnly);
    ds >> connections;
    return connections;
}

void DeepEngine::sync(const QList<ConnectionItem>& connections)
{
    QFile file(QCoreApplication::applicationDirPath() + "/connect.bin");

    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }
    QDataStream ds(&file);
    ds << connections;
    file.close();
}
