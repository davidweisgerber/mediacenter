#ifndef PRESET_H
#define PRESET_H

#include <QWidget>
#include <QMap>
#include "ui_preset.h"

class Preset : public QWidget
{
	Q_OBJECT

public:
	Preset(int number, QWidget *parent = 0);
	~Preset();
	void setActivated( bool activated );
	void setValues( QMap<int, int> values );
	QMap<int, int> getValues();
	void setNumber( int number );
	void setTitle( QString title );
	void setComment( QString comment );
	QString getTitle();
	QString getComment();
	int getNumber();
	
public slots:
	void rename();

protected:
	void mouseReleaseEvent( QMouseEvent * event );

private:
	Ui::PresetClass ui;
	int m_number;
	QMap<int, int> m_values;

signals:
	void activated();
};

#endif // PRESET_H
