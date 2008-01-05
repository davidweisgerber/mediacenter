#include "lightpresets.h"
#include "preset.h"
#include <cmath>


LightPresets::LightPresets(LightBars *bars, QWidget *parent)
	: QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowTitleHint )
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
}

LightPresets::~LightPresets()
{

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
}

void LightPresets::overwritePreset() {
	if( !m_current ) {
		return;
	}

	m_current->setValues( m_bars->getStatus() );
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
	m_bars->setStatus( p->getValues() );
}

void LightPresets::showToggle() {
	setVisible( !isVisible() );
}

int LightPresets::getMaster() {
	return ui.masterSlider->value();
}
