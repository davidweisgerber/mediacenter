#ifndef BEAMERCONTROL_H
#define BEAMERCONTROL_H

#include <QMainWindow>
#include <QSerialPort>
#include <QElapsedTimer>
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
	void input1();
	void input2();
	void input3();
	void input4();
	void initialize();
	void showToggle();

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
    QSerialPort *m_serial;
	int m_lastCommand;
	int m_lastStatus;
	QElapsedTimer m_lastAction;
	QString m_status, m_input, m_temperature, m_lamptime, m_command;

signals:
	void stateChanged( QString state );
	void updateStatus();
	void badThing();
};

#endif // BEAMERCONTROL_H
