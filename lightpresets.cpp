#include "lightpresets.h"
#include "preset.h"
#include <cmath>
#include <QSettings>
#include <QTimer>
#include <QMessageBox>


LightPresets::LightPresets(LightBars *bars, QWidget *parent)
    : QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint )
{
	ui.setupUi(this);
	m_bars = bars;

	m_current = 0;
	sarea = new QScrollArea( this );
	sarea->setGeometry( 190, 5, 605, 91 );
	sarea->setWidgetResizable( false );
	sarea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

	QWidget *base = new QWidget( this );
	layout = new QHBoxLayout();
	layout->setSizeConstraint( QLayout::SetFixedSize );
	layout->setSpacing( 2 );
	layout->setMargin( 1 );

	base->setLayout( layout );
	sarea->setWidget( base );

	connect( ui.masterDial, SIGNAL( valueChanged ( int ) ), this, SLOT( timerChanged( int ) ) );
	connect( ui.masterSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( masterChanged( int ) ) );
	connect( ui.saveNewButton, SIGNAL( clicked() ), this, SLOT( newPreset() ) );
	connect( ui.overWriteButton, SIGNAL( clicked() ), this, SLOT( overwritePreset() ) );
	connect( ui.deleteButton, SIGNAL( clicked() ), this, SLOT( deletePreset() ) );
	connect( ui.blackButton, SIGNAL( clicked() ), this, SLOT( setBlack() ) );
	connect( ui.fullPowerButton, SIGNAL( clicked() ), this, SLOT( setFull() ) );

	restorePresets();
}

LightPresets::~LightPresets()
{
	savePresets();
}

void LightPresets::newPreset() {
	Preset *p = new Preset( layout->count() + 1, sarea->widget() );
	layout->addWidget( p );
	connect( p, SIGNAL( activated() ), this, SLOT( presetActivated() ) );
	p->rename();
	for( int i=0; i < layout->count(); i++ ) {
		Preset *cur = (Preset*)layout->itemAt(i)->widget();
		cur->setActivated( false );
	}
	m_current = p;
	p->setActivated( true );
	p->setValues( m_bars->getStatus() );

	savePresets();
}

void LightPresets::overwritePreset() {
	if( !m_current ) {
		return;
	}

	m_current->setValues( m_bars->getStatus() );

	savePresets();
}

void LightPresets::setBlack() {
	QMap<int, int> map = m_bars->getStatus();
	QMap<int, int> retVal;
	QList<int> channels = map.uniqueKeys();
	for (int i=0; i < channels.size(); ++i) {
		 retVal.insert( channels.at(i), 0 );
	}
	m_bars->setStatus( retVal );
}

void LightPresets::setFull() {
	QMap<int, int> map = m_bars->getStatus();
	QMap<int, int> retVal;
	QList<int> channels = map.uniqueKeys();
	for (int i=0; i < channels.size(); ++i) {
		 retVal.insert( channels.at(i), 100 );
	}
	m_bars->setStatus( retVal );
}

void LightPresets::deletePreset() {
	if( !m_current ) {
		return;
	}

	for( int i=0; i < layout->count(); i++ ) {
		Preset *cur = (Preset*)layout->itemAt(i)->widget();
		if( cur == m_current ) {
			delete cur;
			m_current = 0;
			break;
		}
	}

	for( int i=0; i < layout->count(); i++ ) {
		Preset *cur = (Preset*)layout->itemAt(i)->widget();
		cur->setNumber( i+1 );
	}

	savePresets();
}

void LightPresets::timerChanged( int time ) {
	timerValue = exp( (double)time/10 ) / 100;
	ui.masterTimer->setText( QString::number( timerValue ) + "s" );
}

void LightPresets::masterChanged( int strength ) {
	ui.masterStrength->setText( QString::number( strength ) + "%" );
}

void LightPresets::presetActivated() {
	Preset *p = (Preset*) sender();
	for( int i=0; i < layout->count(); i++ ) {
		Preset *cur = (Preset*)layout->itemAt(i)->widget();
		if( p !=  cur ) {
			cur->setActivated( false );
			cur->update();
		}
	}
	m_current = p;
	m_fadeStart = m_bars->getStatus();
	m_fadeEnd = p->getValues();
	m_fadeCounter.start();
	presetStep();
}

void LightPresets::presetStep() {
	QMap<int, int> status;
	QList<int> channels = m_fadeEnd.uniqueKeys();
	for (int i=0; i < channels.size(); ++i) {
		status[channels.at(i)] = m_fadeStart[channels.at(i)] +
			(double)( m_fadeEnd[channels.at(i)] - m_fadeStart[channels.at(i)] ) *
			(double)( (double)m_fadeCounter.elapsed() / (double)(timerValue * 1000) );
	}

	m_bars->setStatus( status );

	if( m_fadeCounter.elapsed() < timerValue * 1000.0 ) {
		QTimer::singleShot( 25, this, SLOT( presetStep() ) );
	} else {
		m_bars->setStatus( m_fadeEnd );
	}
}

void LightPresets::showToggle() {
	setVisible( !isVisible() );
}

int LightPresets::getMaster() {
	return ui.masterSlider->value();
}

void LightPresets::savePresets() {
	QSettings settings( QSettings::UserScope, "FEGMM", "mediacenter" );
	settings.setValue( "presets", layout->count() );
	settings.setValue( "autofader", ui.masterDial->value() );

	for( int i=0; i < layout->count(); i++ ) {
		settings.beginGroup( "preset" + QString::number( i ) );
		Preset *cur = (Preset*)layout->itemAt(i)->widget();
		settings.setValue( "title", cur->getTitle() );
		settings.setValue( "comment", cur->getComment() );
		
		QMap<int, int> values = cur->getValues();
		QList<int> channels = values.uniqueKeys();
		for (int i=0; i < channels.size(); ++i) {
			settings.setValue( QString::number( channels.at(i) ),
				values.value(channels.at(i), 0) );
		}
		
		settings.endGroup();
	}
}

void LightPresets::restorePresets() {
	QSettings settings( QSettings::UserScope, "FEGMM", "mediacenter" );
	int presets = settings.value( "presets", 0 ).toInt();
	timerChanged( settings.value( "autofader", 0 ).toInt() );
	ui.masterDial->setValue( settings.value( "autofader", 0 ).toInt() );
	
	for( int i=0; i < presets; i++ ) {
		settings.beginGroup( "preset" + QString::number( i ) );
		Preset *p = new Preset( layout->count() + 1, sarea->widget() );
		layout->addWidget( p );
		connect( p, SIGNAL( activated() ), this, SLOT( presetActivated() ) );

		p->setTitle( settings.value( "title" ).toString() );
		p->setComment( settings.value( "comment" ).toString() );

		QMap<int, int> values;
		QStringList keys = settings.childKeys();
		for (int j = 0; j < keys.size(); ++j) {
			bool ok;
			int key = keys.at(j).toInt(&ok);
			if( ok ) {
				values.insert( key, settings.value( keys.at(j), 0 ).toInt() );
			}
		}

		p->setValues( values );
		settings.endGroup();
	}
}
