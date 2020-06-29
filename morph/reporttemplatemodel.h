#ifndef REPORTTEMPLATEMODEL_H
#define REPORTTEMPLATEMODEL_H

#include <QAbstractTableModel>
#include <QScopedPointer>
#include <QJsonArray>
class QFile;

class ReportTemplateModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ReportTemplateModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    // Add data:
    bool addTemplate(const QString& name, const QString& text);

    // Remove data:
    bool removeTemplate(const QString& name);

    // Dynamicly data load
    void fetchMore(const QModelIndex &parent = QModelIndex()) override;
    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const override;

private:
    QScopedPointer<QFile> handler;
    QJsonArray items;
};

#endif // REPORTTEMPLATEMODEL_H
