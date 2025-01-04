#ifndef MEDIACENTER_H
#define MEDIACENTER_H

#include <QJsonObject>
#include "ui_mediacenter.h"
#include "lightbars.h"
#include "lightpresets.h"
#include "beamercontrol.h"

class QSystemTrayIcon;
class ConfigureDMX;
class DMXThread;

class Mediacenter : public QMainWindow
{
	Q_OBJECT

public:
    explicit Mediacenter(QWidget *parent = nullptr);
	~Mediacenter() override;

private slots:
	void showAllControls();
	void hideAllControls();
	void connectDMX();
    void disconnectDMX();
	void beamerStateChange( const QString& state );
	void configureBeamer();

private:
	Ui::mediacenterClass ui{};
	LightBars *m_lightBars = nullptr;
	LightPresets *m_lightPresets = nullptr;
	BeamerControl *m_beamerControl = nullptr;
	ConfigureDMX *m_configureDMX = nullptr;
    QSystemTrayIcon *m_systray = nullptr;

	QString m_beamerStatus;

    QJsonObject m_settingsObject;
    DMXThread *m_dmxThread = nullptr;
    char m_dmxBuffer[512] = {};
};

#endif // MEDIACENTER_H
