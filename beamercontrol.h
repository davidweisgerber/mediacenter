#ifndef BEAMERCONTROL_H
#define BEAMERCONTROL_H

#include <QSerialPort>
#include <QElapsedTimer>
#include "ui_beamercontrol.h"

class BeamerControl : public QMainWindow
{
	Q_OBJECT

public:
	explicit BeamerControl(QWidget *parent = nullptr);
	~BeamerControl() override;

	QString getStatus();
	QString getTemperature();
	QString getLampTime();
	QString getInput();

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
	void processStatus(const QString& buf);
	void processInput(const QString& buf);
	void processLampTime(QString buf);
	void processTemperature(const QString& buf);
	void processCommandFeedback(const QString& buf);
	void proceedCommandPipe();

	Ui::BeamerControlClass ui;
    QSerialPort *m_serial = nullptr;
	int m_lastCommand = 0;
	int m_lastStatus = 0;
	QElapsedTimer m_lastAction;
	QString m_status;
	QString m_input;
	QString m_temperature;
	QString m_lamptime;
	QString m_command;

signals:
	void stateChanged(QString state);
	void updateStatus();
	void badThing();
};

#endif // BEAMERCONTROL_H
