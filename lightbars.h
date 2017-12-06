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
    LightBars(char *dmxBuffer,  QWidget *parent = 0);
	~LightBars();
	QMap<int, int>getStatus();
	void setStatus( QMap<int, int> status );
    bool isFaderMaster(int channel);

public slots:
	void showToggle();
    void buildUp(const QJsonObject &source);
    void masterChanged(int newMaster);

private:
	QVBoxLayout *layout;
    char *m_dmxBuffer;
    int m_master;
};

#endif // LIGHTBARS_H
