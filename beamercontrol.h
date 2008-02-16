#ifndef BEAMERCONTROL_H
#define BEAMERCONTROL_H

#include <QMainWindow>
#include <QExtSerialPort.h>
#include <QTime>
#include "ui_beamercontrol.h"

class BeamerControl : public QMainWindow
{
	Q_OBJECT

public:
	BeamerControl(QWidget *parent = 0);
	~BeamerControl();

	QString getStatus() { return m_status; }
	QString getTemperature() { return m_temperature; }
	QString getLampTime() { return m_lamptime; }
	QString getInput() { return m_input; }

public slots:
	void powerOn();
	void powerOff();

private slots:
	void processSerial();

private:
	void processStatus( QString buf );
	void processInput( QString buf );
	void processLampTime( QString buf );
	void processTemperature( QString buf );
	void processCommandFeedback( QString buf );
	void proceedCommandPipe();
	Ui::BeamerControlClass ui;
	QextSerialPort *m_serial;
	int m_lastCommand;
	int m_lastStatus;
	QTime m_lastAction;
	QString m_status, m_input, m_temperature, m_lamptime, m_command;

signals:
	void stateChanged( QString state );
	void updateStatus();
	void badThing();
};

#endif // BEAMERCONTROL_H
