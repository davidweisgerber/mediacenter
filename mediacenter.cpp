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
#include "dmxthread.h"


mediacenter::mediacenter(QWidget *parent)
    : QMainWindow(parent)
{
    memset(m_dmxBuffer, 0, 512);
    m_dmxThread = new DMXThread(m_dmxBuffer);

    QFile file("settings.json");
    if (file.open(QIODevice::ReadOnly) == false)
    {
        qDebug() << "Could not open" << file.fileName() << "for reading";
    }

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    m_settingsObject = jsonDoc.object();

    lbars = new LightBars(m_dmxBuffer, this);
    lbars->buildUp(m_settingsObject);
	lbars->show();

	lpresets = new LightPresets( lbars, this );
    lpresets->buildUp(m_settingsObject);
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

    m_systray = new QSystemTrayIcon( QIcon( QPixmap( off_xpm ) ), this );
    m_systray->show();

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
    //menu->addAction( tr("Configure DMX Channels"), configDMX, SLOT( show() ) );
    //menu->addAction( tr("Configure Beamer Connection"), this, SLOT( configureBeamer() ) );
	menu->addSeparator();
	menu->addAction( tr("Close"), qApp, SLOT(quit()) );
	
    m_systray->setContextMenu( menu );

    m_dmxThread->connectDMX();
    m_dmxThread->start(QThread::TimeCriticalPriority);
}

mediacenter::~mediacenter()
{
    m_dmxThread->disconnectDMX();
    m_dmxThread->quitThread();
    m_dmxThread->wait();

    delete m_dmxThread;
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

void mediacenter::connectDMX()
{
    if (m_dmxThread->connectDMX() == false)
    {
        QMessageBox::critical(this, tr("Error connecting DMX"), m_dmxThread->getError());
        m_dmxThread->disconnectDMX();
        return;
    }

    m_systray->showMessage( tr("DMX Connected!"), tr("You can control the light now."), QSystemTrayIcon::Information, 5000 );
    m_systray->setToolTip(tr("DMX connected"));
    m_systray->setIcon( QIcon(  QPixmap( on_xpm ) ) );
}

void mediacenter::disconnectDMX()
{
    m_dmxThread->disconnectDMX();

    m_systray->showMessage( tr("DMX Disconnected!"), tr("Controls have been turned off."), QSystemTrayIcon::Information, 5000 );
    m_systray->setToolTip(tr("DMX disconnected"));
    m_systray->setIcon( QIcon(  QPixmap( off_xpm ) ) );
}

void mediacenter::beamerStateChange( QString state ) {
    m_systray->showMessage( tr("Beamer Information"), state, QSystemTrayIcon::Information, 5000 );
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
