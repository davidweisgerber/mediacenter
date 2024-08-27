#include "lightpresets.h"
#include "preset.h"
#include <cmath>
#include <QSettings>
#include <QTimer>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>


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
	layout->setContentsMargins( 1, 1, 1, 1 );

	base->setLayout( layout );
	sarea->setWidget( base );

	connect( ui.masterDial, SIGNAL( valueChanged ( int ) ), this, SLOT( timerChanged( int ) ) );
	connect( ui.masterSlider, SIGNAL( valueChanged ( int ) ), this, SLOT( masterChanged( int ) ) );
	connect( ui.saveNewButton, SIGNAL( clicked() ), this, SLOT( newPreset() ) );
	connect( ui.overWriteButton, SIGNAL( clicked() ), this, SLOT( overwritePreset() ) );
	connect( ui.deleteButton, SIGNAL( clicked() ), this, SLOT( deletePreset() ) );
	connect( ui.blackButton, SIGNAL( clicked() ), this, SLOT( setBlack() ) );
	connect( ui.fullPowerButton, SIGNAL( clicked() ), this, SLOT( setFull() ) );

    m_settingsFile = QDir::homePath() + QDir::separator() + "mediacenter.settings.json";
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
	QList<int> channels = map.keys();
	for (int i=0; i < channels.size(); ++i) {
		 retVal.insert( channels.at(i), 0 );
	}
	m_bars->setStatus( retVal );
}

void LightPresets::setFull() {
	QMap<int, int> map = m_bars->getStatus();
	QMap<int, int> retVal;
	QList<int> channels = map.keys();
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
    m_bars->masterChanged(strength);
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
	QList<int> channels = m_fadeEnd.keys();
	for (int i=0; i < channels.size(); ++i) {
        if (m_bars->isFaderMaster(i))
        {
            status[channels.at(i)] = m_fadeStart[channels.at(i)] +
                (double)( m_fadeEnd[channels.at(i)] - m_fadeStart[channels.at(i)] ) *
                (double)( (double)m_fadeCounter.elapsed() / (double)(timerValue * 1000) );
        }
        else
        {
            status[channels.at(i)] = m_fadeEnd[channels.at(i)];
        }
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

void LightPresets::savePresets()
{
    QJsonArray array;

    for( int i=0; i < layout->count(); i++ )
    {
        Preset *cur = (Preset*)layout->itemAt(i)->widget();
        if (cur->isSystem())
        {
            continue;
        }

        QJsonObject object;
        object["title"] = cur->getTitle();
        object["comment"] = cur->getComment();
		
		QMap<int, int> values = cur->getValues();
		QList<int> channels = values.keys();
        for (int i=0; i < channels.size(); ++i)
        {
            object[QString::number(channels.at(i))] = values.value(channels.at(i), 0);
		}

        array.append(object);
    }

    QByteArray settings = QJsonDocument(array).toJson();
    QFile file(m_settingsFile);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        QMessageBox::critical(this, tr("Error"), tr("Could not save settings to %1").arg(m_settingsFile));
        return;
    }

    file.write(settings);
    file.close();
}

void LightPresets::buildUp(const QJsonObject &source)
{
    QJsonArray faderArray = source["presets"].toArray();
    double timerValue = source["autofader"].toDouble();

    timerValue = log(timerValue * 100) * 10;
    //timerValue = exp( (double)time/10 ) / 100;

    timerChanged(timerValue);
    ui.masterDial->setValue(timerValue);

    addPresets(faderArray, true);

	m_bars->masterChanged(ui.masterSlider->value());

    QFile file(m_settingsFile);
    if (file.open(QIODevice::ReadOnly) == false)
    {
        return;
    }

    QByteArray settings = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(settings);
    addPresets(document.array(), false);
}

void LightPresets::addPresets(const QJsonArray &faderArray, bool isSystem)
{
    int numberOfPresets = faderArray.size();
    for (int i=0; i < numberOfPresets; i++)
    {
        Preset *p = new Preset( layout->count() + 1, sarea->widget() );
        p->setSystem(isSystem);
        layout->addWidget( p );
        connect( p, SIGNAL( activated() ), this, SLOT( presetActivated() ) );

        p->setTitle(faderArray[i].toObject()["title"].toString());
        p->setComment(faderArray[i].toObject()["comment"].toString());

        QMap<int, int> values;

        for (int j=0; j < 512; j++)
        {
            if (faderArray[i].toObject().contains(QString::number(j)))
            {
                values.insert(j, faderArray[i].toObject()[QString::number(j)].toInt());
            }
        }

        p->setValues(values);
    }
}
