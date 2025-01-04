#include "lightpresets.h"
#include "preset.h"
#include <cmath>
#include <QSettings>
#include <QTimer>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>


LightPresets::LightPresets(LightBars *bars, QWidget *parent)
    : QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint )
{
	ui.setupUi(this);
	m_bars = bars;

	m_current = nullptr;
	m_scrollArea = new QScrollArea( this );
	m_scrollArea->setGeometry( 190, 5, 605, 91 );
	m_scrollArea->setWidgetResizable( false );
	m_scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

	auto *base = new QWidget( this );
	m_layout = new QHBoxLayout();
	m_layout->setSizeConstraint( QLayout::SetFixedSize );
	m_layout->setSpacing( 2 );
	m_layout->setContentsMargins( 1, 1, 1, 1 );

	base->setLayout( m_layout );
	m_scrollArea->setWidget( base );

	connect( ui.masterDial, &QDial::valueChanged, this, &LightPresets::timerChanged);
	connect( ui.masterSlider, &QSlider::valueChanged, this, &LightPresets::masterChanged);
	connect( ui.saveNewButton, &QAbstractButton::clicked, this, &LightPresets::newPreset);
	connect( ui.overWriteButton, &QAbstractButton::clicked, this, &LightPresets::overwritePreset);
	connect( ui.deleteButton, &QAbstractButton::clicked, this, &LightPresets::deletePreset);
	connect( ui.blackButton, &QAbstractButton::clicked, this, &LightPresets::setBlack);
	connect( ui.fullPowerButton, &QAbstractButton::clicked, this, &LightPresets::setFull);

	m_httpServer.route("/get/<arg>", this, [this](const int presetNo)
	{
		if (presetNo > m_layout->count())
		{
			return QHttpServerResponse("application/json", "false", QHttpServerResponse::StatusCode::BadRequest);
		}

		auto* presetWidget = qobject_cast<Preset*>(m_layout->itemAt(presetNo-1)->widget());
		QJsonObject object;
		object["title"] = presetWidget->getTitle();
		object["comment"] = presetWidget->getComment();
		object["current"] = presetWidget == m_current;

		return QHttpServerResponse("application/json", QJsonDocument(object).toJson());
	});

	m_httpServer.route("/activate/<arg>", this, [this](const int presetNo)
	{
		if (presetNo > m_layout->count())
		{
			return QHttpServerResponse("application/json", "false", QHttpServerResponse::StatusCode::BadRequest);
		}

		auto* presetWidget = qobject_cast<Preset*>(m_layout->itemAt(presetNo-1)->widget());
		presetWidget->setActivated(true);
		emit presetWidget->activated();

		return QHttpServerResponse("application/json", "true");
	});

    m_settingsFile = QDir::homePath() + QDir::separator() + "mediacenter.settings.json";

	m_tcpServer = new QTcpServer(this);
	if (m_tcpServer->listen(QHostAddress::Any, 8089) == false || m_httpServer.bind(m_tcpServer) == false)
	{
		delete m_tcpServer;

		qCritical() << "Could not bind to port 8089";
	}
}

LightPresets::~LightPresets()
{
	savePresets();
	delete m_tcpServer;
}

void LightPresets::newPreset() {
	auto *p = new Preset( m_layout->count() + 1, m_scrollArea->widget() );
	m_layout->addWidget( p );
	connect( p, &Preset::activated, this, &LightPresets::presetActivated);
	p->rename();

	for(int i=0; i < m_layout->count(); i++)
	{
		auto *cur = qobject_cast<Preset*>(m_layout->itemAt(i)->widget());
		cur->setActivated( false );
	}
	m_current = p;
	p->setActivated( true );
	p->setValues( m_bars->getStatus() );

	savePresets();
}

void LightPresets::overwritePreset() {
	if(m_current == nullptr)
	{
		return;
	}

	m_current->setValues( m_bars->getStatus() );

	savePresets();
}

void LightPresets::setBlack() const
{
	QMap<int, int> map = m_bars->getStatus();
	QMap<int, int> retVal;
	QList<int> channels = map.keys();

	for (int channel : channels)
	{
		 retVal.insert( channel, 0 );
	}
	m_bars->setStatus( retVal );
}

void LightPresets::setFull() const
{
	QMap<int, int> map = m_bars->getStatus();
	QMap<int, int> retVal;
	QList<int> channels = map.keys();

	for (int channel : channels)
	{
		 retVal.insert( channel, 255 );
	}
	m_bars->setStatus(retVal);
}

void LightPresets::deletePreset()
{
	if(m_current == nullptr)
	{
		return;
	}

	for (int i=0; i < m_layout->count(); i++)
	{
		auto *cur = qobject_cast<Preset*>(m_layout->itemAt(i)->widget());
		if (cur == m_current)
		{
			delete cur;
			m_current = nullptr;
			break;
		}
	}

	for( int i=0; i < m_layout->count(); i++ )
	{
		auto *cur = qobject_cast<Preset*>(m_layout->itemAt(i)->widget());
		cur->setNumber( i+1 );
	}

	savePresets();
}

void LightPresets::timerChanged( int time )
{
	m_timerValue = exp(static_cast<double>(time)/10.0) / 100.0;
	ui.masterTimer->setText(QString::number( m_timerValue ) + "s" );
}

void LightPresets::masterChanged(int strength) const
{
	ui.masterStrength->setText( QString::number( strength ) + "%" );
    m_bars->masterChanged(strength);
}

void LightPresets::presetActivated()
{
	auto *p = qobject_cast<Preset*>(sender());

	for( int i=0; i < m_layout->count(); i++ )
	{
		auto *cur = qobject_cast<Preset*>(m_layout->itemAt(i)->widget());
		if (p !=  cur)
		{
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

void LightPresets::presetStep()
{
	QMap<int, int> status;
	const QList<int> channels = m_fadeEnd.keys();
	for (int i=0; i < channels.size(); ++i)
	{
        if (m_bars->isFaderMaster(i))
        {
            status[channels.at(i)] = static_cast<int>(m_fadeStart[channels.at(i)] +
                static_cast<double>(m_fadeEnd[channels.at(i)] - m_fadeStart[channels.at(i)]) *
                (static_cast<double>(m_fadeCounter.elapsed()) / (m_timerValue * 1000.0)));
        }
        else
        {
            status[channels.at(i)] = m_fadeEnd[channels.at(i)];
        }
	}

	m_bars->setStatus( status );

	if(static_cast<double>(m_fadeCounter.elapsed()) < m_timerValue * 1000.0 )
	{
		QTimer::singleShot( 25, this, &LightPresets::presetStep);
	}
	else
	{
		m_bars->setStatus( m_fadeEnd );
    }
}

void LightPresets::showToggle()
{
	setVisible( !isVisible() );
}

int LightPresets::getMaster() const
{
	return ui.masterSlider->value();
}

void LightPresets::savePresets()
{
    QJsonArray array;

    for (int i=0; i < m_layout->count(); i++)
    {
        auto *cur = qobject_cast<Preset*>(m_layout->itemAt(i)->widget());
        if (cur->isSystem())
        {
            continue;
        }

        QJsonObject object;
        object["title"] = cur->getTitle();
        object["comment"] = cur->getComment();
		
		QMap<int, int> values = cur->getValues();
		QList<int> channels = values.keys();
        for (int channel : channels)
        {
            object[QString::number(channel)] = values.value(channel, 0);
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

    timerChanged(static_cast<int>(timerValue));
    ui.masterDial->setValue(static_cast<int>(timerValue));

    addPresets(faderArray, true);

	m_bars->masterChanged(ui.masterSlider->value());

    QFile file(m_settingsFile);
    if (file.open(QIODevice::ReadOnly) == false)
    {
        return;
    }

    const QByteArray settings = file.readAll();
    file.close();

    const QJsonDocument document = QJsonDocument::fromJson(settings);
    addPresets(document.array(), false);
}

void LightPresets::addPresets(const QJsonArray &faderArray, bool isSystem)
{
    const qsizetype numberOfPresets = faderArray.size();
    for (int i=0; i < numberOfPresets; i++)
    {
        auto *p = new Preset( m_layout->count() + 1, m_scrollArea->widget() );
        p->setSystem(isSystem);
        m_layout->addWidget( p );
        connect( p, &Preset::activated, this, &LightPresets::presetActivated);

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
