#include "afxapp.h"
#include "dbgutility.h"

#pragma execution_character_set("utf-8")





void AfxApp::startElapsedTimer()
{
    this->_elaspeTimer.start();
}

qint64 AfxApp::getElapsed()
{
    return this->_elaspeTimer.elapsed();
}



AfxApp::AfxApp()
{

}
