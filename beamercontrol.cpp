#include "beamercontrol.h"
#include <QIODevice>
#include <QMessageBox>


BeamerControl::BeamerControl(QWidget *parent)
	: QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);

	m_serial = new QextSerialPort( "COM3" );
	m_serial->setBaudRate(BaudRateType::BAUD19200);
	m_serial->setDataBits(DataBitsType::DATA_8);
	m_serial->setParity(ParityType::PAR_NONE);
	m_serial->setStopBits(StopBitsType::STOP_1);
	m_serial->setFlowControl(FlowType::FLOW_OFF);
	if( !m_serial->open(QIODevice::ReadWrite) ) {
		QMessageBox::critical( this, tr("Could not open"), tr("Could not open serial port for beamer control.") );
	}
}

BeamerControl::~BeamerControl()
{

}
