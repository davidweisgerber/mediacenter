#include "lightfader.h"

LightFader::LightFader(int channel, int strength, QString name, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect( ui.slider, SIGNAL( valueChanged ( int ) ), this, SLOT( setSliderValue( int ) ) );

	setChannel( channel );
	setValue( strength );
	setName( name );
}

LightFader::~LightFader()
{

}

void LightFader::setSliderValue( int newValue ) {
	ui.faderStrength->setText( QString::number( newValue ) + "%" );
	m_value = newValue;
}

int LightFader::getValue() {
	return m_value;
}

int LightFader::getChannel() {
	return m_channel;
}

void LightFader::setValue( int value ) {
	ui.slider->setValue( value );
	setSliderValue( value );
}

void LightFader::setChannel( int channel ) {
	m_channel = channel;
}

QString LightFader::getName() {
	return ui.faderName->text();
}

void LightFader::setName( QString name ) {
	ui.faderName->setText( name );
}
