#include "splashwndext.h"

#pragma execution_character_set("utf-8")

SplashWndExt::SplashWndExt(const QPixmap &pixmap, Qt::WindowFlags f)
    : QSplashScreen (pixmap, f)
{
    this->_progressBar = new QProgressBar(this);
    auto rect = this->rect();
    this->_progressBar->setGeometry(rect.left()+50, rect.bottom()-50, rect.width()-100, 20);
    this->_progressBar->setRange(0, 0);
    this->_progressBar->setValue(0);
}

SplashWndExt::~SplashWndExt()
{

}

void SplashWndExt::onSigProgress(int cur, int total, const QString &desc)
{
    this->_progressBar->setRange(0, total);
    this->_progressBar->setValue(cur);
    if( !desc.isEmpty()){
        this->showMessage(desc, Qt::AlignCenter|Qt::AlignBottom, Qt::black);
    }
}
