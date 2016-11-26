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
	LightBars(QWidget *parent = 0);
	~LightBars();
	QMap<int, int>getStatus();
	void setStatus( QMap<int, int> status );

public slots:
	void showToggle();
    void buildUp(const QJsonObject &source);

private:
	QVBoxLayout *layout;
};

#endif // LIGHTBARS_H
