#ifndef BEAMERCONTROL_H
#define BEAMERCONTROL_H

#include <QMainWindow>
#include <QExtSerialPort.h>
#include "ui_beamercontrol.h"

class BeamerControl : public QMainWindow
{
	Q_OBJECT

public:
	BeamerControl(QWidget *parent = 0);
	~BeamerControl();

private:
	Ui::BeamerControlClass ui;
	QextSerialPort *m_serial;
};

#endif // BEAMERCONTROL_H
