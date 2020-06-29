#ifndef BATCHMODEL_H
#define BATCHMODEL_H

#include <QAbstractTableModel>
#include <QTimer>

class BatchModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    BatchModel(int interval=0, QObject* parent=nullptr);
    ~BatchModel() override;

    //! 一次性下载所有的数据
    virtual void fetchAll();
    //! 自动分批下载
    virtual void  startAutoFetch();

protected:
    virtual bool _hasMore() = 0;
    virtual void _fetchOnce() = 0;
signals:
    void fetchError(const QString& errString);
    void fetchProgress(int total, int current);

private:
    QTimer      *_fetchTimer;
    int         _fetchInterval;
};

#endif // BATCHMODEL_H
