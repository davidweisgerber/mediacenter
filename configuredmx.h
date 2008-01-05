#ifndef CONFIGUREDMX_H
#define CONFIGUREDMX_H

#include <QDialog>
#include <QSettings>
#include "ui_configuredmx.h"
#include <QListWidgetItem>

class ConfigureDMX : public QDialog
{
	Q_OBJECT

public:
	ConfigureDMX(QWidget *parent = 0);
	~ConfigureDMX();

private slots:
	void addChannel();
	void deleteChannel();
	void currentItemChanged ( QListWidgetItem * current, QListWidgetItem * previous );
	void saveItem( QListWidgetItem *item );
	void setItem( const QListWidgetItem *item );
	void okClicked();

signals:
	void configured();

private:
	bool deleteProc;
	QSettings *settings;
	Ui::ConfigureDMXClass ui;
};

#endif // CONFIGUREDMX_H
