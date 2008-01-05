#ifndef LIGHTFADER_H
#define LIGHTFADER_H

#include <QWidget>
#include "ui_lightfader.h"

class LightFader : public QWidget
{
	Q_OBJECT

public:
	LightFader(int channel, int strength, QString name, QWidget *parent = 0);
	~LightFader();
	int getValue();
	int getChannel();
	void setValue( int value );
	void setChannel( int channel );
	QString getName();
	void setName( QString name );


private slots:
	void setSliderValue( int newValue );

private:
	int m_value;
	int m_channel;
	Ui::LightFaderClass ui;
};

#endif // LIGHTFADER_H
