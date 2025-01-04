#ifndef LIGHTPRESETS_H
#define LIGHTPRESETS_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QScrollArea>
#include "ui_lightpresets.h"
#include "lightbars.h"
#include <QElapsedTimer>
#include <QHttpServer>
#include <QTcpServer>

class Preset;
class QTimer;
class QJsonArray;

class LightPresets : public QMainWindow
{
	Q_OBJECT

public:
	explicit LightPresets(LightBars *bars, QWidget *parent = nullptr);
	~LightPresets() override;
	[[nodiscard]] int getMaster() const;

public slots:
	void showToggle();
	void savePresets();
    void buildUp(const QJsonObject &source);

private slots:
	void timerChanged( int time );
	void masterChanged( int strength ) const;
	void presetActivated();
	void newPreset();
	void overwritePreset();
	void deletePreset();
	void setBlack() const;
	void setFull() const;
	void presetStep();
    void addPresets(const QJsonArray &array, bool isSystem);

private:
	Ui::LightPresetsClass ui{};
	double m_timerValue = 0.0;
	QHBoxLayout *m_layout = nullptr;
	LightBars *m_bars = nullptr;
	QScrollArea *m_scrollArea = nullptr;
	Preset *m_current = nullptr;
	QElapsedTimer m_fadeCounter;
	QMap<int, int> m_fadeStart;
	QMap<int, int> m_fadeEnd;
    QString m_settingsFile;

	QTcpServer *m_tcpServer = nullptr;
	QHttpServer m_httpServer;
};

#endif // LIGHTPRESETS_H
