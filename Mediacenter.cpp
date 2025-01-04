#include "Mediacenter.h"
#include <QSystemTrayIcon>
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QMenu>
#include <QFile>
#include <QJsonDocument>
#include <QtDebug>
#include "debugwindow.h"
#include "configuredmx.h"
#include "dmxthread.h"

Mediacenter::Mediacenter(QWidget *parent)
    :QMainWindow(parent)
{
    memset(m_dmxBuffer, 0, 512);
    m_dmxThread = new DMXThread(m_dmxBuffer);

    QFile file("settings.json");
    if (file.open(QIODevice::ReadOnly) == false)
    {
        qDebug() << "Could not open" << file.fileName() << "for reading";
    }

    QJsonParseError error;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    m_settingsObject = jsonDoc.object();

    m_lightBars = new LightBars(m_dmxBuffer, this);
    m_lightBars->buildUp(m_settingsObject);
	m_lightBars->show();

	m_lightPresets = new LightPresets( m_lightBars, this );
    m_lightPresets->buildUp(m_settingsObject);
	m_lightPresets->show();

    /*bcontrol = new BeamerControl( this );
	connect( bcontrol, SIGNAL( stateChanged( QString ) ), this, SLOT( beamerStateChange( QString ) ) );
	connect( bcontrol, SIGNAL( updateStatus() ), this, SLOT( setSystrayToolTip() ) );
    bcontrol->show();*/
    m_beamerControl = nullptr;

	m_configureDMX = new ConfigureDMX( this );
    m_configureDMX->setWindowIcon(QIcon(QPixmap(QString("://on.xpm"))));

	//connect( m_configureDMX, &ConfigureDMX::configured, m_lightBars, SLOT( buildUp() ) );

	QRect geo = qApp->primaryScreen()->availableGeometry();
	m_lightBars->move( geo.width() - m_lightBars->geometry().size().width() - 6, geo.y() );
	m_lightPresets->move( m_lightBars->geometry().x() - m_lightPresets->geometry().width() - 9, geo.y() );
    /*bcontrol->move( lbars->geometry().x() - bcontrol->geometry().width() - 9,
        lpresets->geometry().y() + lpresets->geometry().height() + 3);*/

	hide();

    m_systray = new QSystemTrayIcon(QIcon(QString("://off.xpm")), this);
    m_systray->show();

	auto *menu = new QMenu(this);
	menu->addAction( tr("Connect DMX"), this, &Mediacenter::connectDMX);
	menu->addAction( tr("Disconnect DMX"), this, &Mediacenter::disconnectDMX);
	menu->addSeparator();
	menu->addAction( tr("Show All"), this, &Mediacenter::showAllControls);
	menu->addAction( tr("Hide All"), this, &Mediacenter::hideAllControls);
	menu->addSeparator();
	menu->addAction( tr("Show/Hide Light Bars"), m_lightBars, &LightBars::showToggle);
	menu->addAction( tr("Show/Hide Light Presets"), m_lightPresets, &LightPresets::showToggle);
    //menu->addAction( tr("Show/Hide Beamer Control"), bcontrol, SLOT( showToggle() ) );
	menu->addSeparator();
    //menu->addAction( tr("Configure DMX Channels"), configDMX, SLOT( show() ) );
    //menu->addAction( tr("Configure Beamer Connection"), this, SLOT( configureBeamer() ) );
	menu->addSeparator();
	menu->addAction( tr("Close"), qApp, &QCoreApplication::quit);
	
    m_systray->setContextMenu(menu);

    m_dmxThread->connectDMX();
    m_dmxThread->start(QThread::TimeCriticalPriority);

#ifdef QT_DEBUG
    auto *debugWindow = new DebugWindow(m_dmxBuffer, this);
    debugWindow->show();
#endif //QT_DEBUG

}

Mediacenter::~Mediacenter()
{
    m_dmxThread->disconnectDMX();
    m_dmxThread->quitThread();
    m_dmxThread->wait();

    delete m_dmxThread;
}

void Mediacenter::showAllControls()
{
	m_lightBars->show();
	m_lightPresets->show();
    //bcontrol->show();
}

void Mediacenter::hideAllControls()
{
	m_lightBars->hide();
	m_lightPresets->hide();
    //bcontrol->hide();
}

void Mediacenter::connectDMX()
{
    if (m_dmxThread->connectDMX() == false)
    {
        QMessageBox::critical(this, tr("Error connecting DMX"), m_dmxThread->getError());
        m_dmxThread->disconnectDMX();
        return;
    }

    m_systray->showMessage( tr("DMX Connected!"), tr("You can control the light now."), QSystemTrayIcon::Information, 5000 );
    m_systray->setToolTip(tr("DMX connected"));
    m_systray->setIcon( QIcon(  QPixmap(QString("://on.xpm")) ) );
}

void Mediacenter::disconnectDMX()
{
    m_dmxThread->disconnectDMX();

    m_systray->showMessage( tr("DMX Disconnected!"), tr("Controls have been turned off."), QSystemTrayIcon::Information, 5000 );
    m_systray->setToolTip(tr("DMX disconnected"));
    m_systray->setIcon( QIcon(  QPixmap(QString("://off.xpm")) ) );
}

void Mediacenter::beamerStateChange( const QString& state )
{
    m_systray->showMessage( tr("Beamer Information"), state, QSystemTrayIcon::Information, 5000 );
}

void Mediacenter::configureBeamer() {
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
