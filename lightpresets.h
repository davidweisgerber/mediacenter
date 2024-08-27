#ifndef LIGHTPRESETS_H
#define LIGHTPRESETS_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QScrollArea>
#include "ui_lightpresets.h"
#include "lightbars.h"
#include <QElapsedTimer>

class Preset;
class QTimer;
class QJsonArray;

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
    void buildUp(const QJsonObject &source);

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
    void addPresets(const QJsonArray &array, bool isSystem);

private:
	Ui::LightPresetsClass ui;
	double timerValue;
	QHBoxLayout *layout;
	LightBars *m_bars;
	QScrollArea *sarea;
	Preset *m_current;
	QElapsedTimer m_fadeCounter;
	QMap<int, int> m_fadeStart, m_fadeEnd;
    QString m_settingsFile;
};

#endif // LIGHTPRESETS_H
