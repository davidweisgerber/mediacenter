#ifndef LIGHTPRESETS_H
#define LIGHTPRESETS_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QScrollArea>
#include "ui_lightpresets.h"
#include "lightbars.h"
#include <QTime>

class Preset;
class QTimer;

class LightPresets : public QMainWindow
{
	Q_OBJECT

public:
	LightPresets(LightBars *bars, QWidget *parent = 0);
	~LightPresets();
	int getMaster();

public slots:
	void showToggle();
	void savePresets();

private slots:
	void timerChanged( int time );
	void masterChanged( int strength );
	void presetActivated();
	void newPreset();
	void overwritePreset();
	void deletePreset();
	void setBlack();
	void setFull();
	void presetStep();

private:
	void restorePresets();
	Ui::LightPresetsClass ui;
	double timerValue;
	QHBoxLayout *layout;
	LightBars *m_bars;
	QScrollArea *sarea;
	Preset *m_current;
	QTime m_fadeCounter;
	QMap<int, int> m_fadeStart, m_fadeEnd;
};

#endif // LIGHTPRESETS_H
