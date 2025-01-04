#include "configuredmx.h"

ConfigureDMX::ConfigureDMX(QWidget *parent)
	:QDialog(parent)
{
	m_deleteProc = false;
	ui.setupUi(this);
	m_settings = new QSettings( QSettings::SystemScope, "FEGMM", "mediacenter" );

	const int num_faders = m_settings->value( "faders", 0 ).toInt();

	for (int i=0; i < num_faders; i++)
	{
		m_settings->beginGroup( "fader" + QString::number(i) );
		const int channel = m_settings->value( "channel", 0 ).toInt();
		QString name = m_settings->value( "name", "" ).toString();

		ui.listWidget->addItem( QString::number( channel ) + " (" + name + ")" );

		m_settings->endGroup();
	}

	if (ui.listWidget->count() > 0)
	{
		setItem( ui.listWidget->item( 0 ) );
		ui.listWidget->setCurrentRow( 0 );
	}

	connect(ui.listWidget, &QListWidget::currentItemChanged, this, &ConfigureDMX::currentItemChanged);
	connect(ui.addButton, &QAbstractButton::clicked, this, &ConfigureDMX::addChannel);
	connect(ui.deleteButton, &QAbstractButton::clicked, this, &ConfigureDMX::deleteChannel);
	connect(ui.okButton, &QAbstractButton::clicked, this, &ConfigureDMX::okClicked);
	connect(ui.upButton, &QAbstractButton::clicked, this, &ConfigureDMX::moveChannelUp);
	connect(ui.downButton, &QAbstractButton::clicked, this, &ConfigureDMX::moveChannelDown);
}

ConfigureDMX::~ConfigureDMX()
{
	delete m_settings;
}

void ConfigureDMX::addChannel()
{

	ui.listWidget->addItem( tr("New channel") );
	ui.listWidget->setCurrentRow( ui.listWidget->count() - 1 );

	ui.channelSpinBox->setValue( 0 );
	ui.name->setText( tr("New channel") );
	ui.defaultSpinBox->setValue( 0 );

	m_settings->setValue( "faders", ui.listWidget->count() );
}

void ConfigureDMX::currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous)
{
	if (m_deleteProc)
	{
		return;
	}

	if (previous)
	{
		saveItem( previous );
	}

	if (current)
	{
		if(current->text() != tr("New channel"))
		{
			setItem( current );
		}
	}
}

void ConfigureDMX::saveItem( QListWidgetItem *item )
{
	int num = ui.listWidget->row(item);

	m_settings->beginGroup( "fader" + QString::number(num) );
	m_settings->setValue( "channel", ui.channelSpinBox->value() );
	m_settings->setValue( "strength", ui.defaultSpinBox->value() );
	m_settings->setValue( "name", ui.name->text() );
	m_settings->endGroup();

	item->setText( QString::number( ui.channelSpinBox->value() ) + " (" + ui.name->text() + ")" );
}

void ConfigureDMX::setItem( const QListWidgetItem *item )
{
	int num = ui.listWidget->row(item);

	m_settings->beginGroup( "fader" + QString::number(num) );

	ui.channelSpinBox->setValue( m_settings->value( "channel", 0 ).toInt() );
	ui.name->setText( m_settings->value( "name", "" ).toString() );
	ui.defaultSpinBox->setValue( m_settings->value( "strength", 0 ).toInt() );

	m_settings->endGroup();
}

void ConfigureDMX::okClicked()
{
	QListWidgetItem *current = ui.listWidget->currentItem();

	if(current)
	{
		saveItem( current );
	}

	hide();
	emit configured();
}


void ConfigureDMX::deleteChannel()
{
	QListWidgetItem *current = ui.listWidget->currentItem();

	if (!current)
	{
		return;
	}

	int num = ui.listWidget->row(current);
	delete current;

	for (int i=num; i < ui.listWidget->count(); i++)
	{
		m_settings->beginGroup( "fader" + QString::number(i+1) );

		ui.channelSpinBox->setValue( m_settings->value( "channel", 0 ).toInt() );
		ui.name->setText( m_settings->value( "name", "" ).toString() );
		ui.defaultSpinBox->setValue( m_settings->value( "strength", 0 ).toInt() );

		m_settings->endGroup();

		saveItem( ui.listWidget->item( i ) );
	}

	m_deleteProc = true;
	ui.listWidget->setCurrentRow( 0 );
	setItem( ui.listWidget->item( 0 ) );
	m_deleteProc = false;

	m_settings->setValue( "faders", ui.listWidget->count() );
}

void ConfigureDMX::moveChannelUp()
{
	QListWidgetItem *current = ui.listWidget->currentItem();

	if (!current)
	{
		return;
	}

	int num = ui.listWidget->row(current);
	if (num == 0)
	{
		return;
	}

	int tempChannel;
	QString tempText;
	int tempDefault;

	m_settings->beginGroup( "fader" + QString::number(num-1) );

	tempChannel = m_settings->value( "channel", 0 ).toInt();
	tempText = m_settings->value( "name", "" ).toString();
	tempDefault = m_settings->value( "strength", 0 ).toInt();

	m_settings->setValue( "channel", ui.channelSpinBox->value() );
	m_settings->setValue( "strength", ui.defaultSpinBox->value() );
	m_settings->setValue( "name", ui.name->text() );

	m_settings->endGroup();

	m_settings->beginGroup( "fader" + QString::number(num) );

	m_settings->setValue( "channel", tempChannel );
	m_settings->setValue( "strength", tempDefault );
	m_settings->setValue( "name", tempText );

	m_settings->endGroup();

	ui.listWidget->item( num - 1 )->setText( current->text() );
	current->setText( QString::number( tempChannel ) + " (" + tempText + ")" );

	m_deleteProc = true;
	ui.listWidget->setCurrentRow( num-1 );
	setItem( ui.listWidget->item( num-1 ) );
	m_deleteProc = false;
}

void ConfigureDMX::moveChannelDown()
{
	QListWidgetItem *current = ui.listWidget->currentItem();

	if (!current)
	{
		return;
	}

	int num = ui.listWidget->row( current );

	if (num >= ( ui.listWidget->count() - 1 ))
	{
		return;
	}

	int tempChannel;
	QString tempText;
	int tempDefault;

	m_settings->beginGroup( "fader" + QString::number(num+1) );

	tempChannel = m_settings->value( "channel", 0 ).toInt();
	tempText = m_settings->value( "name", "" ).toString();
	tempDefault = m_settings->value( "strength", 0 ).toInt();

	m_settings->setValue( "channel", ui.channelSpinBox->value() );
	m_settings->setValue( "strength", ui.defaultSpinBox->value() );
	m_settings->setValue( "name", ui.name->text() );

	m_settings->endGroup();

	m_settings->beginGroup( "fader" + QString::number(num) );

	m_settings->setValue( "channel", tempChannel );
	m_settings->setValue( "strength", tempDefault );
	m_settings->setValue( "name", tempText );

	m_settings->endGroup();

	ui.listWidget->item( num + 1 )->setText( current->text() );
	current->setText( QString::number( tempChannel ) + " (" + tempText + ")" );

	m_deleteProc = true;
	ui.listWidget->setCurrentRow( num+1 );
	setItem( ui.listWidget->item( num+1 ) );
	m_deleteProc = false;
}
