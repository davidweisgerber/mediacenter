#include "mediacenter.h"
#include "off.xpm"
#include "on.xpm"
#include <QSystemTrayIcon>
#include <QApplication>
#include <QDesktopWidget>
#include "configuredmx.h"

mediacenter::mediacenter(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{

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
}

void mediacenter::setSystrayToolTip() {
	systray->setToolTip( "Media Control Status\n\n" + dmxStatus + "\n" + beamerStatus );
}

mediacenter::~mediacenter()
{

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

	dmxStatus = tr("DMX disconnected");
	systray->showMessage( tr("DMX Connected!"), tr("You can control the light now."), QSystemTrayIcon::Information, 5000 );
	setSystrayToolTip();
}

void mediacenter::disconnectDMX() {

	dmxStatus = tr("DMX disconnected");
	systray->showMessage( tr("DMX Disconnected!"), tr("Controls have been turned off."), QSystemTrayIcon::Information, 5000 );
	setSystrayToolTip();
}
