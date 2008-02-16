#include "beamercontrol.h"
#include <QIODevice>
#include <QTimer>
#include <QMessageBox>


BeamerControl::BeamerControl(QWidget *parent)
	: QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);

	m_serial = new QextSerialPort( "COM4" );
	m_serial->setBaudRate(BaudRateType::BAUD19200);
	m_serial->setDataBits(DataBitsType::DATA_8);
	m_serial->setParity(ParityType::PAR_NONE);
	m_serial->setStopBits(StopBitsType::STOP_1);
	m_serial->setFlowControl(FlowType::FLOW_OFF);
	m_lastCommand = 0;
	m_lastStatus = -1;
	if( !m_serial->open(QIODevice::ReadWrite) ) {
		QMessageBox::critical( this, tr("Could not open"), tr("Could not open serial port for beamer control.") );
	}

	connect( ui.onButton, SIGNAL( clicked() ), this, SLOT( powerOn() ) );
	connect( ui.offButton, SIGNAL( clicked() ), this, SLOT( powerOff() ) );

	processSerial();
}

BeamerControl::~BeamerControl()
{

}

void BeamerControl::powerOn() {
	m_command = "C00\r\n";
}

void BeamerControl::powerOff() {
	m_command = "C01\r\n";
}

void BeamerControl::processSerial() {
	char buf[10];
	int size;
	if( 0 != (size = m_serial->read( buf, 10 ) ) ) {
		buf[size] = 0;
		switch( m_lastCommand ) {
			case 0:
				break;
			case 1:
				processStatus( buf );
				break;
			case 2:
				processInput( buf );
				break;
			case 3:
				processLampTime( buf );
				break;
			case 4:
				processTemperature( buf );
				break;
			case 5:
				processCommandFeedback( buf );
				break;
		}
	}

	if( m_lastAction.isNull() || m_lastAction.elapsed() > 3000 ) {
		proceedCommandPipe();
		m_lastAction.start();
	}

	QTimer::singleShot( 1000, this, SLOT( processSerial() ) );
}

void BeamerControl::processStatus( QString buf ) {
	bool ok;
	int status = buf.toInt( &ok );
	if( status != m_lastStatus && ok ) {
		m_lastStatus = status;
		QString statusString;
		bool bad = false;

		switch( status ) {
			case 0:
				statusString = tr("Power On");
				break;
			case 80:
				statusString = tr("Standby");
				break;
			case 40:
				statusString = tr("Turning On");
				break;
			case 20:
				statusString = tr("Cooling Down");
				break;
			case 10:
				statusString = tr("Power Failure");
				bad = true;
				break;
			case 28:
				statusString = tr("Cooling Down due to Abnormal Temperature in Progress");
				bad = true;
				break;
			case 24:
				statusString = tr("Power Management Cooling Down in Progress");
				break;
			case 04:
				statusString = tr("Power Management Status");
				break;
			case 21:
				statusString = tr("Cooling Down after Lamp Failure");
				bad = true;
				break;
			case 81:
				statusString = tr("Standby after Lamp Failure");
				bad = true;
				break;
		}
		m_status = statusString;
		
		if( bad ) {
			emit badThing();
		} else {
			emit stateChanged( statusString );
		}

		emit updateStatus();

		ui.statusLabel->setText( "Status: " + m_status );

	}

	proceedCommandPipe();
}

void BeamerControl::proceedCommandPipe() {
	if( !m_command.isEmpty() ) {
		m_serial->write( m_command.toLatin1() );
		m_lastCommand = 5;
		m_command = "";
	} else {
		switch( m_lastCommand ) {
			default:
				m_serial->write("CR0\r\n");
				m_lastCommand = 1;
				break;
		}
	}
}



void BeamerControl::processInput( QString buf ) {
	
}

void BeamerControl::processLampTime( QString buf ) {
	
}

void BeamerControl::processTemperature( QString buf ) {
	
}

void BeamerControl::processCommandFeedback( QString buf ) {
	
}

