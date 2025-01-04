#ifndef CONFIGUREDMX_H
#define CONFIGUREDMX_H

#include <QSettings>
#include "ui_configuredmx.h"
#include <QListWidgetItem>

class ConfigureDMX : public QDialog
{
	Q_OBJECT

public:
	explicit ConfigureDMX(QWidget *parent = nullptr);
	~ConfigureDMX() override;

private slots:
	void addChannel();
	void deleteChannel();
	void currentItemChanged ( QListWidgetItem * current, QListWidgetItem * previous );
	void saveItem( QListWidgetItem *item );
	void setItem( const QListWidgetItem *item );
	void okClicked();
	void moveChannelUp();
	void moveChannelDown();

signals:
	void configured();

private:
	bool m_deleteProc = false;
	QSettings *m_settings = nullptr;
	Ui::ConfigureDMXClass ui{};
};

#endif // CONFIGUREDMX_H
