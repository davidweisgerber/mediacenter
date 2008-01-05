#ifndef BEAMERCONTROL_H
#define BEAMERCONTROL_H

#include <QMainWindow>
#include "ui_beamercontrol.h"

class BeamerControl : public QMainWindow
{
	Q_OBJECT

public:
	BeamerControl(QWidget *parent = 0);
	~BeamerControl();

private:
	Ui::BeamerControlClass ui;
};

#endif // BEAMERCONTROL_H
