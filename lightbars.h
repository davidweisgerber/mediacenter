#ifndef LIGHTBARS_H
#define LIGHTBARS_H

#include <QMainWindow>
#include <QMap>
#include "lightfader.h"

class QVBoxLayout;

class LightBars : public QMainWindow
{
	Q_OBJECT

public:
    explicit LightBars(char *dmxBuffer,  QWidget *parent = nullptr);
	~LightBars() override;
	QMap<int, int>getStatus();
	void setStatus(const QMap<int, int>& status);
    bool isFaderMaster(int channel);

public slots:
	void showToggle();
    void buildUp(const QJsonObject &source);
    void masterChanged(int newMaster);

private:
	QVBoxLayout *m_layout = nullptr;
    char *m_dmxBuffer = nullptr;
    int m_master = 0;
};

#endif // LIGHTBARS_H
