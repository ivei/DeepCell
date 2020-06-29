#ifndef DEEPENGINE_H
#define DEEPENGINE_H

#include <QObject>

#include "frame/connectionitem.h"
class DeepEngine : public QObject
{
    Q_OBJECT
public:
    static DeepEngine& instance()
    {
        static DeepEngine inst;
        return inst;
    }

    QList<ConnectionItem> getConnections();
    void sync(const QList<ConnectionItem>&);

signals:

private:
    DeepEngine();
};

#endif // DEEPENGINE_H
