#ifndef DMXTHREAD_H
#define DMXTHREAD_H

#include <QThread>
#include "FTD2XX.H"

class DMXThread : public QThread
{
public:
    explicit DMXThread(char *dmxBuffer);

    bool connectDMX();
    void disconnectDMX();
    const QString &getError();
    void quitThread();

protected:
    void run() override;

private:
    bool m_dmxConnected;
    bool m_running;
    FT_HANDLE m_ftHandle;
    QString m_error;
    char *m_dmxBuffer;
};

#endif // DMXTHREAD_H
