#include "reporttemplatemodel.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#pragma execution_character_set("utf-8")

ReportTemplateModel::ReportTemplateModel(QObject *parent)
    : QAbstractTableModel(parent), handler(nullptr)
{
}

QVariant ReportTemplateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
          return tr("报告模板");
    }

    return  QVariant();
}

int ReportTemplateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return items.size();
}

int ReportTemplateModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 1;
}

QVariant ReportTemplateModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        auto obj = items.at(index.row()).toObject();
        return obj.value("name").toString();
    }

    if (role == Qt::UserRole)
    {
        auto obj = items.at(index.row()).toObject();
        return obj.value("value").toString();
    }

    return QVariant();
}

bool ReportTemplateModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

bool ReportTemplateModel::addTemplate(const QString& name, const QString& text)
{
    auto iter = std::find_if(items.begin(), items.end(), [name](const QJsonValue& value) { return value.toObject().value("name") == name; });
    if (iter != items.end())
    {
        return false;
    }
    auto row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    items.append(QJsonObject({{"name", name}, {"value", text}}));
    endInsertRows();
    handler->reset();
    handler->write(QJsonDocument(items).toJson());
    return true;
}

bool ReportTemplateModel::removeTemplate(const QString &name)
{
    auto iter = std::find_if(items.begin(), items.end(), [name](const QJsonValue& value) { return value.toObject().value("name") == name; });
    if (iter == items.end())
    {
        return false;
    }

    auto row = int(std::distance(items.begin(), iter));
    beginRemoveRows(QModelIndex(), row, row);
    items.erase(iter);
    endRemoveRows();
    handler->reset();
    handler->write(QJsonDocument(items).toJson());
    return true;
}

bool ReportTemplateModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return handler.isNull();
}

void ReportTemplateModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    auto file = new QFile("d:/xxx.json");
    if (file->open(QFile::ReadWrite))
    {
        auto doc = QJsonDocument::fromJson(file->readAll());
        beginResetModel();
        items = doc.array();
        endResetModel();
        handler.reset(file);
    }
}



