#include "mediacenter.h"
#include "off.xpm"
#include "on.xpm"
#include <QSystemTrayIcon>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QTimer>
#include "configuredmx.h"


mediacenter::mediacenter(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	dmxConnected = false;
	lbars = new LightBars( this );
	lbars->show();

	lpresets = new LightPresets( lbars, this );
	lpresets->show();

	bcontrol = new BeamerControl( this );
	//bcontrol->show();

	configDMX = new ConfigureDMX( this );
	configDMX->setWindowIcon( QIcon( on_xpm ) );

	connect( configDMX, SIGNAL( configured() ), lbars, SLOT( buildUp() ) );

	QRect geo = qApp->desktop()->availableGeometry();
	lbars->move( geo.width() - lbars->geometry().size().width() - 6, geo.y() );
	lpresets->move( lbars->geometry().x() - lpresets->geometry().width() - 9, geo.y() );

	hide();

	systray = new QSystemTrayIcon( QIcon( off_xpm ), this );
	systray->show();

	QMenu *menu = new QMenu();
	menu->addAction( tr("Connect DMX"), this, SLOT( connectDMX() ) );
	menu->addAction( tr("Disconnect DMX"), this, SLOT( disconnectDMX() ) );
	menu->addSeparator();
	menu->addAction( tr("Show All"), this, SLOT(showAllControls()) );
	menu->addAction( tr("Hide All"), this, SLOT(hideAllControls()) );
	menu->addSeparator();
	menu->addAction( tr("Show/Hide Light Bars"), lbars, SLOT( showToggle() ) );
	menu->addAction( tr("Show/Hide Light Presets"), lpresets, SLOT( showToggle() ) );
	menu->addAction( tr("Show/Hide Beamer Control"), bcontrol, SLOT( showToggle() ) );
	menu->addSeparator();
	menu->addAction( tr("Configure DMX Channels"), configDMX, SLOT( show() ) );
	menu->addSeparator();
	menu->addAction( tr("Close"), qApp, SLOT(quit()) );
	
	systray->setContextMenu( menu );

	dmxStatus = tr("DMX disconnected");
	beamerStatus = tr("Beamer disconnected");
	setSystrayToolTip();

	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), this, SLOT( sendDMX() ) );
	timer->start( 40 );
}

void mediacenter::setSystrayToolTip() {
	systray->setToolTip( "Media Control Status\n\n" + dmxStatus + "\n" + beamerStatus );
}

mediacenter::~mediacenter()
{
	disconnectDMX();
}

void mediacenter::showAllControls() {
	lbars->show();
	lpresets->show();
	bcontrol->show();
}

void mediacenter::hideAllControls() {
	lbars->hide();
	lpresets->hide();
	bcontrol->hide();
}

void mediacenter::connectDMX() {
	if( dmxConnected ) {
		return;
	}

	FT_STATUS ftStatus;
    char Buf[64];

	ftStatus = FT_ListDevices(0,Buf,FT_LIST_BY_INDEX|FT_OPEN_BY_DESCRIPTION);

    ftHandle = FT_W32_CreateFile(Buf,GENERIC_READ|GENERIC_WRITE,0,0,
        OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL | FT_OPEN_BY_DESCRIPTION,0);

    // connect to first device
    if (ftHandle == INVALID_HANDLE_VALUE) {
		QMessageBox::critical( this, tr("Error at connecting"),
			tr("No DMX device was found"));
        return;
    }

    FTDCB ftDCB;
    if (FT_W32_GetCommState(ftHandle,&ftDCB)) {
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

        if (!FT_W32_SetCommState(ftHandle,&ftDCB)) {
            QMessageBox::critical( this, tr("Error at connecting"),
				tr("The baud rate of the DMX device could not be set."));
            return;
        }
	} else {
		QMessageBox::critical( this, tr("Error at connecting"),
				tr("The DMX device could not be initialized"));
	}

    FT_W32_PurgeComm(ftHandle,FT_PURGE_TX | FT_PURGE_RX);

    dmxConnected = true;

	dmxStatus = tr("DMX connected");
	systray->showMessage( tr("DMX Connected!"), tr("You can control the light now."), QSystemTrayIcon::Information, 5000 );
	setSystrayToolTip();
}

void mediacenter::disconnectDMX() {
	if( !dmxConnected ) {
		return;
	}

	FT_W32_CloseHandle(ftHandle);
	dmxConnected = false;
	dmxStatus = tr("DMX disconnected");
	systray->showMessage( tr("DMX Disconnected!"), tr("Controls have been turned off."), QSystemTrayIcon::Information, 5000 );
	setSystrayToolTip();
}

void mediacenter::sendDMX() {
	if( !dmxConnected ) {
		return;
	}

	unsigned char DMXData[512];
    unsigned char StartCode = 0;
	ULONG bytesWritten;

	QMap<int, int> status = lbars->getStatus();
	for( int i = 0; i < 512; i++ ) {
		float value = status[i];
		value = value * lpresets->getMaster() / 100;
		value = value * 2.55;
		DMXData[i] = (unsigned char)( value );
	}

	FT_W32_EscapeCommFunction(ftHandle,CLRRTS);
	FT_W32_SetCommBreak(ftHandle);
    FT_W32_ClearCommBreak(ftHandle);
	FT_W32_WriteFile(ftHandle, &StartCode, 1, &bytesWritten, NULL);
    FT_W32_WriteFile(ftHandle, DMXData, 512, &bytesWritten, NULL);
}