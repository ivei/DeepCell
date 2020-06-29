#ifndef AFXAPPCONTAINER_H
#define AFXAPPCONTAINER_H

#include <QElapsedTimer>

class AfxApp
{
public:
    static AfxApp& getInstance(){
        static AfxApp _instance;
        return _instance;
    }
    void startElapsedTimer();
    qint64 getElapsed();


private:

    AfxApp();

    QElapsedTimer   _elaspeTimer;

};

#endif // AFXAPPCONTAINER_H
