#ifndef DEEPSERVICE_H
#define DEEPSERVICE_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

class DeepService
{
public:
    enum class PathType : quint8{
        ResourceFile,       //! 普通的资源文件的路径
        ActionContour,      //! 细胞轮廓的路径

    };
    DeepService();
    static void makeUserDirs(const QString& username);
    static QString getReportFilePath(const QString& username, const QString &slide_name);
    //! 根据资源文件的路径转换为保存在本地的cache文件路径
    static QString makeResourceCacheName(const QString& server_path);
    //! 构造细胞轮廓cache文件的本地路径
    static QString makeContourCacheName(const QString& slide_no, const QString& contour_id);
    //! 构造视野cache文件的本地路径
    static QString makeImageListCacheName(const QString& slide_no);
    //!
    static bool cacheRawData(const QString& file_name, const QByteArray& data);
    static QByteArray loadRawData(const QString& file_name);
    static QJsonDocument loadJsonDoc(const QString& file_name);

#if 0   // 重构待删除
    //! 将服务器地址转换为本地地址.
    static std::pair<QString, QString> server2CachePath(const QString& server_path, PathType path_type);
    //! 在cache中查找server_path的内容.
    static QByteArray readCache(const QString& server_path, PathType path_type);
    static bool writeCache(const QString& server_path, const QByteArray& data, PathType path_type);
    //! 保存细胞轮廓数据到文件中.
    static bool cacheContour(const QString& slide_no, const QString& contour_id, const QByteArray& data);
    static QByteArray loadContour(const QString& slide_no, const QString& contour_id);


    static bool cacheImageList(const QString& slide_no, const QByteArray& data);
    static QByteArray loadImageList(const QString& slide_no);
#endif
};

#endif // DEEPSERVICE_H
