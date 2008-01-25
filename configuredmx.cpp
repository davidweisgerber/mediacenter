#include "configuredmx.h"

ConfigureDMX::ConfigureDMX(QWidget *parent)
	: QDialog(parent)
{
	deleteProc = false;
	ui.setupUi(this);
	settings = new QSettings( QSettings::SystemScope, "FEGMM", "mediacenter" );
	
	int num_faders = settings->value( "faders", 0 ).toInt();

	for( int i=0; i < num_faders; i++ ) {
		settings->beginGroup( "fader" + QString::number(i) );
		int channel = settings->value( "channel", 0 ).toInt();
		QString name = settings->value( "name", "" ).toString();

		ui.listWidget->addItem( QString::number( channel ) + " (" + name + ")" );

		settings->endGroup();
	}

	if( ui.listWidget->count() > 0 ) {
		setItem( ui.listWidget->item( 0 ) );
		ui.listWidget->setCurrentRow( 0 );
	}

	connect( ui.listWidget, SIGNAL( currentItemChanged ( QListWidgetItem *, QListWidgetItem * ) ), this,
		SLOT( currentItemChanged ( QListWidgetItem *, QListWidgetItem * ) ) );
	connect( ui.addButton, SIGNAL( clicked() ), this, SLOT( addChannel() ) );
	connect( ui.deleteButton, SIGNAL( clicked() ), this, SLOT( deleteChannel() ) );
	connect( ui.okButton, SIGNAL( clicked() ), this, SLOT( okClicked() ) );
	connect( ui.upButton, SIGNAL( clicked() ), this, SLOT( moveChannelUp() ) );
	connect( ui.downButton, SIGNAL( clicked() ), this, SLOT( moveChannelDown() ) );
}

ConfigureDMX::~ConfigureDMX()
{

}

void ConfigureDMX::addChannel() {

	ui.listWidget->addItem( tr("New channel") );
	ui.listWidget->setCurrentRow( ui.listWidget->count() - 1 );

	ui.channelSpinBox->setValue( 0 );
	ui.name->setText( tr("New channel") );
	ui.defaultSpinBox->setValue( 0 );

	settings->setValue( "faders", ui.listWidget->count() );
}

void ConfigureDMX::currentItemChanged( QListWidgetItem * current, QListWidgetItem * previous ) {
	if( deleteProc ) {
		return;
	}
	if( previous ) {
		saveItem( previous );
	}

	if( current ) {
		if( current->text() != tr("New channel") ) {
			setItem( current );
		}
	}
}

void ConfigureDMX::saveItem( QListWidgetItem *item ) {
	int num = ui.listWidget->row(item);

	settings->beginGroup( "fader" + QString::number(num) );
	settings->setValue( "channel", ui.channelSpinBox->value() );
	settings->setValue( "strength", ui.defaultSpinBox->value() );
	settings->setValue( "name", ui.name->text() );
	settings->endGroup();

	item->setText( QString::number( ui.channelSpinBox->value() ) + " (" + ui.name->text() + ")" );
}

void ConfigureDMX::setItem( const QListWidgetItem *item ) {
	int num = ui.listWidget->row(item);

	settings->beginGroup( "fader" + QString::number(num) );

	ui.channelSpinBox->setValue( settings->value( "channel", 0 ).toInt() );
	ui.name->setText( settings->value( "name", "" ).toString() );
	ui.defaultSpinBox->setValue( settings->value( "strength", 0 ).toInt() );

	settings->endGroup();
}

void ConfigureDMX::okClicked() {
	QListWidgetItem *current = ui.listWidget->currentItem();

	if( current ) {
		saveItem( current );
	}

	hide();
	emit configured();
}


void ConfigureDMX::deleteChannel() {
	QListWidgetItem *current = ui.listWidget->currentItem();

	if( !current ) {
		return;
	}

	int num = ui.listWidget->row( current );
	delete current;

	for( int i=num; i < ui.listWidget->count(); i++ ) {
		settings->beginGroup( "fader" + QString::number(i+1) );

		ui.channelSpinBox->setValue( settings->value( "channel", 0 ).toInt() );
		ui.name->setText( settings->value( "name", "" ).toString() );
		ui.defaultSpinBox->setValue( settings->value( "strength", 0 ).toInt() );

		settings->endGroup();

		saveItem( ui.listWidget->item( i ) );
	}

	deleteProc = true;
	ui.listWidget->setCurrentRow( 0 );
	setItem( ui.listWidget->item( 0 ) );
	deleteProc = false;

	settings->setValue( "faders", ui.listWidget->count() );
}

void ConfigureDMX::moveChannelUp() {
	QListWidgetItem *current = ui.listWidget->currentItem();

	if( !current ) {
		return;
	}

	int num = ui.listWidget->row( current );
	if( num == 0 ) {
		return;
	}

	int tempChannel;
	QString tempText;
	int tempDefault;

	settings->beginGroup( "fader" + QString::number(num-1) );

	tempChannel = settings->value( "channel", 0 ).toInt();
	tempText = settings->value( "name", "" ).toString();
	tempDefault = settings->value( "strength", 0 ).toInt();

	settings->setValue( "channel", ui.channelSpinBox->value() );
	settings->setValue( "strength", ui.defaultSpinBox->value() );
	settings->setValue( "name", ui.name->text() );

	settings->endGroup();

	settings->beginGroup( "fader" + QString::number(num) );
	
	settings->setValue( "channel", tempChannel );
	settings->setValue( "strength", tempDefault );
	settings->setValue( "name", tempText );

	settings->endGroup();

	ui.listWidget->item( num - 1 )->setText( current->text() );
	current->setText( QString::number( tempChannel ) + " (" + tempText + ")" );

	deleteProc = true;
	ui.listWidget->setCurrentRow( num-1 );
	setItem( ui.listWidget->item( num-1 ) );
	deleteProc = false;
}

void ConfigureDMX::moveChannelDown() {
	QListWidgetItem *current = ui.listWidget->currentItem();

	if( !current ) {
		return;
	}

	int num = ui.listWidget->row( current );
	if( num >= ( ui.listWidget->count() - 1 ) ) {
		return;
	}

	int tempChannel;
	QString tempText;
	int tempDefault;

	settings->beginGroup( "fader" + QString::number(num+1) );

	tempChannel = settings->value( "channel", 0 ).toInt();
	tempText = settings->value( "name", "" ).toString();
	tempDefault = settings->value( "strength", 0 ).toInt();

	settings->setValue( "channel", ui.channelSpinBox->value() );
	settings->setValue( "strength", ui.defaultSpinBox->value() );
	settings->setValue( "name", ui.name->text() );

	settings->endGroup();

	settings->beginGroup( "fader" + QString::number(num) );
	
	settings->setValue( "channel", tempChannel );
	settings->setValue( "strength", tempDefault );
	settings->setValue( "name", tempText );

	settings->endGroup();

	ui.listWidget->item( num + 1 )->setText( current->text() );
	current->setText( QString::number( tempChannel ) + " (" + tempText + ")" );

	deleteProc = true;
	ui.listWidget->setCurrentRow( num+1 );
	setItem( ui.listWidget->item( num+1 ) );
	deleteProc = false;
}
