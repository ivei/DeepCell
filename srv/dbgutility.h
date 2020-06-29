#ifndef DBGUTILITY_H
#define DBGUTILITY_H
#include <QDebug>

#define CHString(string)    (QString::fromLocal8bit(string))

#define TRACE()    qDebug() << "<TRACE>[" << __FUNCTION__ << "(...)]"
#define INFO()     qDebug() << "<INFO>[" << __FUNCTION__ << "(...)]"
#define WARN()     qDebug() << "<WARN>[" << __FUNCTION__ << "(...)]"
#define ERROR()    qDebug() << "<ERROR>[" << __FUNCTION__ << "(...)]"
#define CLOCK()    qDebug() << "<CLOCK>[" << __FUNCTION__ << "(...)]"

#define IF_THEN(condition, ...)     {       \
        if (condition){__VA_ARGS__ ;}        \
    }

#define IF_RETURN(condition)    do{     \
        if(condition){  return; }       \
    }while(0)

#define IF_RETURN_2(condition, msg)    do{            \
        if(condition){  TRACE() << msg; return; }       \
    }while(0)


#define IF_RETURN_VAL(condition, val)   do{     \
    if(condition){return val;}                  \
    }while(0)

#define IF_RETURN_VAL_2(condition, val, msg)   do{     \
    if(condition){TRACE() << msg;  return val;}                  \
    }while(0)

#define  START_CLOCK(msg)   {CLOCK() << msg; AfxApp::getInstance().startElapsedTimer();}
#define  PRINT_CLOCK(msg)  CLOCK() << msg << AfxApp::getInstance().getElapsed()
#define  GET_CLOCK()    AfxApp::getInstance().getElapsed()


#define RETURN_VAL_IF(rule, val) { if(rule) return val;}
#endif // DBGUTILITY_H
