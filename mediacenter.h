#ifndef MEDIACENTER_H
#define MEDIACENTER_H

#include <QMainWindow>
#include "ui_mediacenter.h"
#include "lightbars.h"
#include "lightpresets.h"
#include "beamercontrol.h"
#include <windows.h>
#include "FTD2XX.h"

class QSystemTrayIcon;
class ConfigureDMX;

class mediacenter : public QMainWindow
{
	Q_OBJECT

public:
	mediacenter(QWidget *parent = 0, Qt::WFlags flags = 0);;
	~mediacenter();

private slots:
	void showAllControls();
	void hideAllControls();
	void connectDMX();
	void disconnectDMX();
	void sendDMX();
	void setSystrayToolTip();
	void beamerStateChange( QString state );

private:
	Ui::mediacenterClass ui;
	LightBars *lbars;
	LightPresets *lpresets;
	BeamerControl *bcontrol;
	ConfigureDMX *configDMX;
	QSystemTrayIcon *systray;

	bool dmxConnected;
	FT_HANDLE ftHandle;

	QString dmxStatus;
	QString beamerStatus;
};

#endif // MEDIACENTER_H
