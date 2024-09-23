#include "dmxthread.h"

DMXThread::DMXThread(char *dmxBuffer) : QThread()
  , m_dmxConnected(false)
  , m_running(true)
  , m_ftHandle(reinterpret_cast<FT_HANDLE>(INVALID_HANDLE_VALUE))
  , m_error()
  , m_dmxBuffer(dmxBuffer)
{

}

bool DMXThread::connectDMX()
{
#ifdef Q_OS_WIN
    if(m_dmxConnected == true)
    {
        return true;
    }

    wchar_t buf[64];
    FT_ListDevices(0, buf, FT_LIST_BY_INDEX | FT_OPEN_BY_DESCRIPTION);

    m_ftHandle = FT_W32_CreateFile(buf, GENERIC_READ|GENERIC_WRITE, 0, 0,
        OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FT_OPEN_BY_DESCRIPTION, 0);

    // connect to first device
    if (m_ftHandle == reinterpret_cast<FT_HANDLE>(INVALID_HANDLE_VALUE))
    {
        m_error = tr("No DMX device was found");
        return false;
    }

    FTDCB ftDCB;
    if (FT_W32_GetCommState(m_ftHandle, &ftDCB))
    {
        // FT_W32_GetCommState ok, device state is in ftDCB
        ftDCB.BaudRate = 250000;
        ftDCB.Parity = FT_PARITY_NONE;
        ftDCB.StopBits = FT_STOP_BITS_2;
        ftDCB.ByteSize = FT_BITS_8;
        ftDCB.fOutX = false;
        ftDCB.fInX = false;
        ftDCB.fErrorChar = false;
        ftDCB.fBinary = true;
        ftDCB.fRtsControl = false;
        ftDCB.fAbortOnError = false;

        if (!FT_W32_SetCommState(m_ftHandle,&ftDCB))
        {
            m_error = tr("The baud rate of the DMX device could not be set.");
            return false;
        }
    }
    else
    {
        m_error = tr("The DMX device could not be initialized");
        return false;
    }

    FT_W32_PurgeComm(m_ftHandle, FT_PURGE_TX | FT_PURGE_RX);

    m_dmxConnected = true;

    return true;
#endif //Q_OS_WIN
#ifdef Q_OS_LINUX
	return false;
#endif //Q_OS_LINUX
}

void DMXThread::disconnectDMX()
{
    if (m_dmxConnected == false)
    {
        return;
    }

    m_dmxConnected = false;
    QThread::msleep(50);

#ifdef Q_OS_WIN
    FT_W32_CloseHandle(m_ftHandle);
#endif //Q_OS_WIN
}

const QString &DMXThread::getError()
{
    return m_error;
}

void DMXThread::quitThread()
{
    m_running = false;
}

void DMXThread::run()
{
    while (m_running == true)
    {
        QThread::msleep(20);

        if(m_dmxConnected == false)
        {
            continue;
        }

        unsigned char startCode = 0;
        ULONG bytesWritten;

#ifdef Q_OS_WIN
        FT_W32_EscapeCommFunction(m_ftHandle, CLRRTS);
        FT_W32_SetCommBreak(m_ftHandle);
        FT_W32_ClearCommBreak(m_ftHandle);
        FT_W32_WriteFile(m_ftHandle, &startCode, 1, &bytesWritten, NULL);
        FT_W32_WriteFile(m_ftHandle, m_dmxBuffer, 512, &bytesWritten, NULL);
#endif //Q_OS_WIN
    }
}
