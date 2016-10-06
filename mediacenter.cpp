#include "mediacenter.h"
#include "off.xpm"
#include "on.xpm"
#include <QSystemTrayIcon>
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QMenu>
#include <QFile>
#include <QJsonDocument>
#include <QtDebug>
#include "configuredmx.h"


mediacenter::mediacenter(QWidget *parent)
    : QMainWindow(parent)
{
	dmxConnected = false;

    QFile file("settings.json");
    if (file.open(QIODevice::ReadOnly) == false)
    {
        qDebug() << "Could not open" << file.fileName() << "for reading";
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    m_settingsObject = jsonDoc.object();

	lbars = new LightBars( this );
	lbars->show();

	lpresets = new LightPresets( lbars, this );
	lpresets->show();

    /*bcontrol = new BeamerControl( this );
	connect( bcontrol, SIGNAL( stateChanged( QString ) ), this, SLOT( beamerStateChange( QString ) ) );
	connect( bcontrol, SIGNAL( updateStatus() ), this, SLOT( setSystrayToolTip() ) );
    bcontrol->show();*/
    bcontrol = nullptr;

	configDMX = new ConfigureDMX( this );
    configDMX->setWindowIcon( QIcon( QPixmap(on_xpm) ) );

	connect( configDMX, SIGNAL( configured() ), lbars, SLOT( buildUp() ) );

	QRect geo = qApp->desktop()->availableGeometry();
	lbars->move( geo.width() - lbars->geometry().size().width() - 6, geo.y() );
	lpresets->move( lbars->geometry().x() - lpresets->geometry().width() - 9, geo.y() );
    /*bcontrol->move( lbars->geometry().x() - bcontrol->geometry().width() - 9,
        lpresets->geometry().y() + lpresets->geometry().height() + 3);*/

	hide();

    systray = new QSystemTrayIcon( QIcon( QPixmap( off_xpm ) ), this );
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
    //menu->addAction( tr("Show/Hide Beamer Control"), bcontrol, SLOT( showToggle() ) );
	menu->addSeparator();
	menu->addAction( tr("Configure DMX Channels"), configDMX, SLOT( show() ) );
	menu->addAction( tr("Configure Beamer Connection"), this, SLOT( configureBeamer() ) );
	menu->addSeparator();
	menu->addAction( tr("Close"), qApp, SLOT(quit()) );
	
	systray->setContextMenu( menu );

	dmxStatus = tr("DMX disconnected");
	setSystrayToolTip();

	QTimer *timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), this, SLOT( sendDMX() ) );
	timer->start( 40 );
}

void mediacenter::setSystrayToolTip() {
    systray->setToolTip( dmxStatus );
}

mediacenter::~mediacenter()
{
	disconnectDMX();
}

void mediacenter::showAllControls() {
	lbars->show();
	lpresets->show();
    //bcontrol->show();
}

void mediacenter::hideAllControls() {
	lbars->hide();
	lpresets->hide();
    //bcontrol->hide();
}

void mediacenter::connectDMX() {
	if( dmxConnected ) {
		return;
	}

	FT_STATUS ftStatus;
    wchar_t Buf[64];

	ftStatus = FT_ListDevices(0,Buf,FT_LIST_BY_INDEX|FT_OPEN_BY_DESCRIPTION);

    ftHandle = FT_W32_CreateFile(Buf, GENERIC_READ|GENERIC_WRITE,0,0,
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
    systray->setIcon( QIcon(  QPixmap( on_xpm ) ) );
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
    systray->setIcon( QIcon(  QPixmap( off_xpm ) ) );
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


void mediacenter::beamerStateChange( QString state ) {
	systray->showMessage( tr("Beamer Information"), state, QSystemTrayIcon::Information, 5000 );
}

void mediacenter::configureBeamer() {
    /*int current = 2;
	bool ok;

	QSettings settings( QSettings::SystemScope, "FEGMM", "mediacenter" );
	QString port = settings.value( "beamerport", "COM3" ).toString();

	if( port == "COM1" ) {
		current = 0;
	} else if( port == "COM2" ) {
		current = 1;
	} else if( port == "COM3" ) {
		current = 2;
	} else if( port == "COM4" ) {
		current = 3;
	} else if( port == "COM5" ) {
		current = 4;
	}

	QString comPort = QInputDialog::getItem( this, tr("Select COM Port"), tr("Beamer Port: "),
		QStringList() << "COM1" << "COM2" << "COM3" << "COM4" << "COM5", current, false, &ok );

	if( ok ) {
		settings.setValue( "beamerport", comPort );
		bcontrol->initialize();
    }*/
}
