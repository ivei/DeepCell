
#include "../srv/dbgutility.h"

#include "batchmodel.h"
#pragma execution_character_set("utf-8")

BatchModel::BatchModel(int interval, QObject *parent)
    : QAbstractTableModel(parent)
    , _fetchTimer(new QTimer)
    , _fetchInterval(interval)
{
}

BatchModel::~BatchModel()
{
    TRACE();
}

void BatchModel::fetchAll()
{
    while (this->_hasMore()) {
        this->_fetchOnce();
    }
}

void BatchModel::startAutoFetch()
{
    TRACE();
    _fetchTimer->setSingleShot(true);
    _fetchTimer->callOnTimeout([&](){
        this->_fetchOnce();
        if( !this->_hasMore()){
            TRACE() << "no more data to fetch. ";
            this->_fetchTimer->stop();
        }
        else {
            TRACE() << "more data to fetch. ";
            this->_fetchTimer->setSingleShot(true);
            this->_fetchTimer->start(this->_fetchInterval);
        }
    });

    this->_fetchTimer->start(0);
}
