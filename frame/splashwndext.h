#ifndef SPLASHWNDEXT_H
#define SPLASHWNDEXT_H

#include <QObject>
#include <QWidget>
#include <QSplashScreen>
#include <QProgressBar>
#include <QString>

class SplashWndExt : public QSplashScreen
{
    Q_OBJECT
public:
    SplashWndExt(const QPixmap& pixmap, Qt::WindowFlags f=Qt::WindowStaysOnTopHint);
    ~SplashWndExt() override;
public slots:
    void onSigProgress(int cur, int total, const QString& desc);
private:
    QProgressBar    *_progressBar;
};

#endif // SPLASHWNDEXT_H
