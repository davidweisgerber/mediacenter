#ifndef LIGHTPRESETS_H
#define LIGHTPRESETS_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QScrollArea>
#include "ui_lightpresets.h"
#include "lightbars.h"

class Preset;

class LightPresets : public QMainWindow
{
	Q_OBJECT

public:
	LightPresets(LightBars *bars, QWidget *parent = 0);
	~LightPresets();

public slots:
	void showToggle();

private slots:
	void timerChanged( int time );
	void masterChanged( int strength );
	void presetActivated();
	void newPreset();
	void overwritePreset();
	void deletePreset();
	void setBlack();
	void setFull();

private:
	Ui::LightPresetsClass ui;
	double timerValue;
	QHBoxLayout *layout;
	LightBars *m_bars;
	QScrollArea *sarea;
	Preset *m_current;
};

#endif // LIGHTPRESETS_H
