#ifndef MEDIACENTER_H
#define MEDIACENTER_H

#include <QMainWindow>
#include <QJsonObject>
#include "ui_mediacenter.h"
#include "lightbars.h"
#include "lightpresets.h"
#include "beamercontrol.h"

class QSystemTrayIcon;
class ConfigureDMX;
class DMXThread;

class mediacenter : public QMainWindow
{
	Q_OBJECT

public:
    mediacenter(QWidget *parent = 0);
	~mediacenter();

private slots:
	void showAllControls();
	void hideAllControls();
	void connectDMX();
    void disconnectDMX();
	void beamerStateChange( QString state );
	void configureBeamer();

private:
	Ui::mediacenterClass ui;
	LightBars *lbars;
	LightPresets *lpresets;
	BeamerControl *bcontrol;
	ConfigureDMX *configDMX;
    QSystemTrayIcon *m_systray;

	QString beamerStatus;

    QJsonObject m_settingsObject;
    DMXThread *m_dmxThread;
    char m_dmxBuffer[512] = {0};
};

#endif // MEDIACENTER_H
