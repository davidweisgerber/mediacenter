#include "beamercontrol.h"
#include <QIODevice>
#include <QTimer>
#include <QMessageBox>
#include <QSettings>

BeamerControl::BeamerControl(QWidget *parent)
	: QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowTitleHint)
{
	ui.setupUi(this);

	m_serial = 0;

	connect( ui.onButton, SIGNAL( clicked() ), this, SLOT( powerOn() ) );
	connect( ui.offButton, SIGNAL( clicked() ), this, SLOT( powerOff() ) );
	connect( ui.input1Button, SIGNAL( clicked() ), this, SLOT( input1() ) );
	connect( ui.input2Button, SIGNAL( clicked() ), this, SLOT( input2() ) );
	connect( ui.input3Button, SIGNAL( clicked() ), this, SLOT( input3() ) );
	connect( ui.input4Button, SIGNAL( clicked() ), this, SLOT( input4() ) );


	initialize();
	processSerial();
}

BeamerControl::~BeamerControl()
{

}

void BeamerControl::initialize() {
	if( m_serial ) {
		delete m_serial;
	}

	QSettings settings( QSettings::SystemScope, "FEGMM", "mediacenter" );
	m_serial = new QextSerialPort( settings.value( "beamerport", "COM3" ).toString() );
	m_serial->setBaudRate(BaudRateType::BAUD19200);
	m_serial->setDataBits(DataBitsType::DATA_8);
	m_serial->setParity(ParityType::PAR_NONE);
	m_serial->setStopBits(StopBitsType::STOP_1);
	m_serial->setFlowControl(FlowType::FLOW_OFF);
	m_lastCommand = 0;
	m_lastStatus = -2;
	if( !m_serial->open(QIODevice::ReadWrite) ) {
		QMessageBox::critical( this, tr("Could not open"), tr("Could not open serial port for beamer control.") );
	}
}


void BeamerControl::powerOn() {
	m_command = "C00\r\n";
}

void BeamerControl::powerOff() {
	m_command = "C01\r\n";
}

void BeamerControl::input1() {
	m_command = "C05\r\n";
}

void BeamerControl::input2() {
	m_command = "C06\r\n";
}

void BeamerControl::input3() {
	m_command = "C07\r\n";
}

void BeamerControl::input4() {
	m_command = "C08\r\n";
}

void BeamerControl::processSerial() {
	char buf[100];
	int size;
	if( 0 != (size = m_serial->read( buf, 100 ) ) ) {
		m_lastAction.start();
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

		m_serial->flush();
	}

	if( m_lastAction.elapsed() > 3000 ) {
		processStatus("-1");
		m_lastCommand = 0;
		proceedCommandPipe();
		m_lastAction.start();
	}

	if( m_lastAction.isNull() ) {
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
			case -1:
				statusString = tr("Beamer disconnected");
				break;
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
			case 1:
				m_serial->write("CR1\r\n");
				m_lastCommand = 2;
				break;
			case 2:
				m_serial->write("CR3\r\n");
				m_lastCommand = 3;
				break;
			case 3: 
				m_serial->write("CR6\r\n");
				m_lastCommand = 4;
				break;
			default:
				m_serial->write("CR0\r\n");
				m_lastCommand = 1;
				break;
		}
	}
}



void BeamerControl::processInput( QString buf ) {
	int input = buf.toInt();
	QString inputText;
	switch( input ) {
		case 1:
			inputText = tr("Input 1: VGA or DVI");
			break;
		case 3:
			inputText = tr("Input 3: Video (Component or S-Video)");
			break;
		case 2:
		default:
			inputText = tr("Input 2: BNC (Y, 3 Components or VGA)");
			break;
	}

	m_input = inputText;
	ui.inputLabel->setText( inputText );

	proceedCommandPipe();
}

void BeamerControl::processLampTime( QString buf ) {
	buf += tr("h");
	m_lamptime = buf;
	ui.lampLabel->setText( tr("Lamp life time: ") + buf );

	proceedCommandPipe();
}

void BeamerControl::processTemperature( QString buf ) {
	m_temperature = buf;
	ui.temperatureLabel->setText( tr("Temperature: ") + buf );

	proceedCommandPipe();
}

void BeamerControl::processCommandFeedback( QString buf ) {
	if( buf == "?" ) {
		QMessageBox::critical( this, tr("Beamer Problem"), tr("Could not process the last command.") );
	}

	proceedCommandPipe();
}

void BeamerControl::showToggle() {
	setVisible( !isVisible() );
}
