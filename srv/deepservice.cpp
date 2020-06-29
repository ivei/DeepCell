#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include "deepservice.h"
#include "dbgutility.h"

#pragma execution_character_set("utf-8")

DeepService::DeepService()
{

}

void DeepService::makeUserDirs(const QString &username)
{
    QDir baseDir(QCoreApplication::applicationDirPath());
    baseDir.mkpath(QString("users/%1/report").arg(username));
    baseDir.mkpath(QString("users/%1/cache").arg(username));
}

QString DeepService::getReportFilePath(const QString& username, const QString &slide_name)
{
    return QString("%1/users/%2/report/%3.pdf").arg(QCoreApplication::applicationDirPath()).arg(username).arg(slide_name);
}

/**
 * @brief 服务器上的资源文件转换为本地路径名
 * @param server_path
 * @return
 */
QString DeepService::makeResourceCacheName(const QString &server_path)
{
    // 将服务器路径转换为本地路径
    QString local_path = server_path;
    auto pos = local_path.lastIndexOf("/data/input/");
    if(pos<0){
        return QString();
    }
    local_path.replace(pos, 12, QString("%1/cache/").arg(QCoreApplication::applicationDirPath()));
    // 替换文件后缀名
    QFileInfo info(local_path);
    QString suffix = QFileInfo(local_path).suffix();
    pos = local_path.lastIndexOf(suffix);
    if( pos < 0){
        return QString();
    }
    local_path.replace(pos, suffix.length(), "dat");
    return local_path;
}

/**
 * @brief 构造缓存细胞轮廓数据的缓存文件名
 * @param slide_no
 * @param contour_id
 * @return
 */
QString DeepService::makeContourCacheName(const QString &slide_no, const QString &contour_id)
{
    return QString("%1/cache/%2/contour/%3.dat")
            .arg(QCoreApplication::applicationDirPath())
            .arg(slide_no)
            .arg(contour_id);
}

/**
 * @brief 构造缓存玻片视野列表数据的本地文件名
 * @param slide_no
 * @return
 */
QString DeepService::makeImageListCacheName(const QString &slide_no)
{
    return QString("%1/cache/%2/%3.dat")
            .arg(QCoreApplication::applicationDirPath())
            .arg(slide_no)
            .arg(slide_no)
            ;
}

/**
 * @brief 将数据保存到本地文件中.
 * @param file_name
 * @param data
 */
bool DeepService::cacheRawData(const QString &path_name, const QByteArray &data)
{
    QFileInfo info(path_name);
    QString dir_name = info.path();
    QString file_name = info.fileName();
    if( file_name.isEmpty()){
        ERROR() << QString("path_name is a pure path: %1").arg(path_name);
        return false;
    }
    // 创建目录
    if( !QDir(QCoreApplication::applicationDirPath()).mkpath(dir_name)){
        ERROR() << QString("Fail to create cache dir %1")
                   .arg(dir_name);
        return false;
    }

    // 创建文件
    QFile file(path_name);
    if( !file.open(QFile::WriteOnly/*, QFileDevice::AutoCloseHandle*/)){
        ERROR() << QString("Failed to open cache file %1. error is: %2")
                   .arg(file_name).arg(file.errorString());
        return false;
    }

    // 写数据
    if( !file.write(data)){
        ERROR() << QString("Failed to write data to file %1. error is: %2")
                   .arg(file_name).arg(file.errorString());
        file.close();
        return false;
    }
    file.close();
    return true;
}

/**
 * @brief 读数据文件
 * @param file_name
 * @return
 */
QByteArray DeepService::loadRawData(const QString &file_path)
{
    QFile file(file_path);
    QByteArray data;
    if(!file.open(QFile::ReadOnly)){
        return QByteArray();
    }
    return file.readAll();
}

QJsonDocument DeepService::loadJsonDoc(const QString &file_name)
{
    QByteArray data = loadRawData(file_name);
    if( data.isEmpty()){
        return QJsonDocument();
    }
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if( parseError.error != QJsonParseError::NoError){
        ERROR() << QString("parse file %1 to json failed. error is: %2")
                   .arg(file_name).arg(parseError.errorString());
        return QJsonDocument();
    }
    return doc;
}



#if 0
//! 返回: 目录名, 文件全路径
std::pair<QString, QString> DeepService::server2CachePath(const QString &server_path, PathType path_type)
{
    QString path = server_path;
    if( path_type==PathType::ResourceFile)
    {
        auto pos = path.lastIndexOf("/data/input/");
        if( pos==-1){
            return std::make_pair(QString(), QString());
        }
        path.replace(pos, 12, QString("%1/cache/").arg(QCoreApplication::applicationDirPath()));
        // 替换文件后缀名.
        QFileInfo info(path);
        QString suffix = QFileInfo(path).suffix();
        // 约定一定有后缀名! 下载文件一定有后缀名.
        pos = path.lastIndexOf(suffix);
        path.replace(pos, suffix.length(), "dat");

        return std::make_pair(QFileInfo(path).path(), path);
    }
    else if(path_type==PathType::ActionContour){

    }
}

QByteArray DeepService::readCache(const QString &server_path, PathType path_type)
{
    //TRACE() << "server_path is: " << server_path;
    // 将server_path转换为cache路径
    auto local_path = DeepService::server2CachePath(server_path, path_type);
    QFile file(local_path.second);
    QByteArray result;
    if( file.open(QFile::ReadOnly))
    {
        //TRACE() << "read data";
        result = file.readAll();
        file.close();
        return result;
    }
    else {
        //INFO() << "open cache resource file failed: " << local_path.second << file.errorString();
        file.close();
        return QByteArray();
    }
}

bool DeepService::writeCache(const QString &server_path, const QByteArray &data, PathType path_type)
{
    //TRACE() << "server_path: " << server_path;
    auto local_path = DeepService::server2CachePath(server_path, path_type);
    QDir base(QCoreApplication::applicationDirPath());
    if( !base.mkpath(local_path.first)){
        ERROR() << "Fail to create cache dir";
        return false;
    }
    //TRACE() << "dir maked!";
    QFile file(local_path.second);
    if( !file.open(QIODevice::WriteOnly)){
        ERROR() << "Fail to open cache file to write: " << local_path.second << file.errorString();
        return false;
    }
    if( !file.write(data)){
        ERROR() << "Fail to write data to file " << local_path.second << file.errorString();
        file.close();
        return false;
    }
    else {
        file.close();
        return true;
    }
}



bool DeepService::cacheContour(const QString &slide_no, const QString &contour_id, const QByteArray &data)
{
    // 创建目录
    QString dir_name = QString("%1/cache/%2/contour/").arg(QCoreApplication::applicationDirPath()).arg(slide_no);
    QDir(QCoreApplication::applicationDirPath()).mkpath(dir_name);
    // 创建文件名
    QString file_name = QString("%1/cache/%2/contour/%3.dat")
            .arg(QCoreApplication::applicationDirPath())
            .arg(slide_no)
            .arg(contour_id);

    QFile file(file_name);
    if( !file.open(QFile::WriteOnly))
    {
        ERROR() << "Failed create contour data file. " << file_name << file.errorString();
        return false;
    }
    if( file.write(data) < 0)
    {
        ERROR() << QString("Failed to write data (size is %1) to file %2").arg(data.size()).arg(file_name) << file.errorString();
        file.close();
        return false;
    }
    else {
        file.close();
        return true;
    }
}

QByteArray DeepService::loadContour(const QString &slide_no, const QString &contour_id)
{
    QString file_name = QString("%1/cache/%2/contour/%3.dat")
            .arg(QCoreApplication::applicationDirPath())
            .arg(slide_no)
            .arg(contour_id);
    QFile file(file_name);
    if( !file.open(QFile::ReadOnly))
    {
        //INFO() << "failed to open contour file " << file_name << file.errorString();
        return QByteArray();
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}

/** 保存ImageList的数据
 * @brief DeepService::cacheImageList
 * @param slide_no
 * @param data
 * @return
 */
bool DeepService::cacheImageList(const QString &slide_no, const QByteArray& data)
{
    // 创建目录. image数据就保存在/cache/slide_no/目录下面, 以slide_no.data为文件名
    QString dir_name = QString("%1/cache/%2/").arg(QCoreApplication::applicationDirPath()).arg(slide_no);
    QDir(QCoreApplication::applicationDirPath()).mkpath(dir_name);
    QString file_name = QString("%1/cache/%2/%3.dat")
            .arg(QCoreApplication::applicationDirPath())
            .arg(slide_no)
            .arg(slide_no)
            ;

    QFile file(file_name);
    if( !file.open(QFile::WriteOnly)){
        ERROR() << "Failed create image list cache file. " << file_name << file.errorString();
        return false;
    }

    if( file.write(data)<0){
        ERROR() << QString("Failed write image list data to cachefile %1. error is: %2")
                   .arg(file_name).arg(file.errorString());
        file.close();
        return false;
    }
    file.close();
    return true;
}


QByteArray DeepService::loadImageList(const QString &slide_no)
{
    QString file_name = QString("%1/cache/%2/%3.dat")
            .arg(QCoreApplication::applicationDirPath())
            .arg(slide_no)
            .arg(slide_no)
            ;
    QFile file(file_name);
    if( !file.open(QFile::ReadOnly) ){
        return QByteArray();
    }
    QByteArray data = file.readAll();
    file.close();
    return data;
}
#endif
