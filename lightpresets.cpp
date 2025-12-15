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
#include <QNetworkReply>
#include <QRandomGenerator>

LightPresets::LightPresets(LightBars *bars, QWidget *parent)
    :QMainWindow(parent, Qt::WindowStaysOnTopHint | Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint)
	,m_networkAccessManager(new QNetworkAccessManager(this))
{
	ui.setupUi(this);
	m_bars = bars;

	m_current = nullptr;
	m_scrollArea = new QScrollArea( this );
	m_scrollArea->setGeometry( 190, 5, 605, 91 );
	m_scrollArea->setWidgetResizable( false );
	m_scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );

	auto *base = new QWidget( this ); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
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
	for (int channel : channels)
	{
        if (m_bars->isFaderMaster(channel))
        {
            status[channel] = static_cast<int>(m_fadeStart[channel] +
                static_cast<double>(m_fadeEnd[channel] - m_fadeStart[channel]) *
                (static_cast<double>(m_fadeCounter.elapsed()) / (m_timerValue * 1000.0)));
        }
        else
        {
            status[channel] = m_fadeEnd[channel];
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

	m_tcpServer = new QTcpServer(this);
	if (m_tcpServer->listen(QHostAddress::Any, source["apiPort"].toInt(8089)) == false || m_httpServer.bind(m_tcpServer) == false)
	{
		delete m_tcpServer;
		m_tcpServer = nullptr;

		qCritical() << "Could not bind to port 8089";
	}

	buildButtons(source);
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

void LightPresets::setupButtonCommunicationShelly(const QJsonObject& settings, QPushButton* onButton, QPushButton* offButton)
{
	auto testTimer = new QTimer(this); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
	testTimer->setInterval(1000);
	onButton->setEnabled(false);
	offButton->setEnabled(false);

	auto parseAnswer = [onButton, offButton](const QByteArray &responseData)
	{
		const QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
		const QJsonObject jsonObj = jsonDoc.object();

		if (jsonObj.contains("result") == false)
		{
			qCritical() << "No result from request" << responseData;
			onButton->setEnabled(false);
			offButton->setEnabled(false);
			return;
		}

		const bool isOn = jsonObj["result"].toObject()["output"].toBool();

		onButton->setEnabled(isOn == false);
		offButton->setEnabled(isOn == true);
	};

	connect(testTimer, &QTimer::timeout, this, [parseAnswer, settings, this]()
	{
		QJsonObject requestObject;
		requestObject["id"] = settings["switch"].toInt();
		requestObject["method"] = "Switch.GetStatus";
		QJsonObject paramsObject;
		paramsObject["id"] = settings["switch"].toInt();
		requestObject["params"] = paramsObject;

		QNetworkRequest request(QString("http://%1/rpc").arg(settings["address"].toString()));
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

		sendShellyRequest(request, requestObject, parseAnswer, settings["password"].toString());
	});

	connect(onButton, &QPushButton::clicked, this, [settings, this]()
	{
		QJsonObject requestObject;
		requestObject["id"] = settings["switch"].toInt();
		requestObject["method"] = "Switch.Set";
		QJsonObject paramsObject;
		paramsObject["id"] = settings["switch"].toInt();
		paramsObject["on"] = true;
		requestObject["params"] = paramsObject;

		QNetworkRequest request(QString("http://%1/rpc").arg(settings["address"].toString()));
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

		sendShellyRequest(request, requestObject, [](const QByteArray &){}, settings["password"].toString());
	});

	connect(offButton, &QPushButton::clicked, this, [settings, this]()
	{
		QJsonObject requestObject;
		requestObject["id"] = settings["switch"].toInt();
		requestObject["method"] = "Switch.Set";
		QJsonObject paramsObject;
		paramsObject["id"] = settings["switch"].toInt();
		paramsObject["on"] = false;
		requestObject["params"] = paramsObject;

		QNetworkRequest request(QString("http://%1/rpc").arg(settings["address"].toString()));
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

		sendShellyRequest(request, requestObject, [](const QByteArray &){}, settings["password"].toString());
	});

	testTimer->start();
}
void LightPresets::sendShellyRequest(const QNetworkRequest& request, const QJsonObject& requestObject, const std::function<void(const QByteArray&)>& parseAnswer, const QString &password)
{
	QNetworkReply* reply = m_networkAccessManager->post(request, QJsonDocument(requestObject).toJson());
	connect(reply, &QNetworkReply::finished, this, [reply, requestObject, parseAnswer, password, this, request]()
	{
		if (reply->error() == QNetworkReply::AuthenticationRequiredError)
		{
			const QString authenticate = reply->rawHeader("WWW-Authenticate");
			QString realm;
			QString nonce;
			const QStringList parts = authenticate.split(",");
			if (const auto hasRealm = std::find_if(parts.cbegin(), parts.cend(), [&](const QString& s){return s.trimmed().startsWith("realm");}); hasRealm != parts.cend())
			{
				realm = hasRealm->split("=").last().remove("\"");
			}
			if (const auto hasNonce = std::find_if(parts.cbegin(), parts.cend(), [&](const QString& s){return s.trimmed().startsWith("nonce");}); hasNonce != parts.cend())
			{
				nonce = hasNonce->split("=").last().remove("\"");
			}

			if (realm.isEmpty() == false && nonce.isEmpty() == false)
			{
				constexpr int nc = 1;

				QJsonObject authenticationObject;
				authenticationObject["realm"] = realm;
				authenticationObject["nonce"] = nonce;
				authenticationObject["username"] = "admin";
				authenticationObject["cnonce"] = QString::number(QRandomGenerator::global()->generate64());
				authenticationObject["algorithm"] ="SHA-256";

				QCryptographicHash ha1(QCryptographicHash::Sha256);
				ha1.addData(QString("%1:%2:%3").arg("admin", realm, password).toLatin1());
				const QString ha1Result = QString::fromLatin1(ha1.result().toHex());

				QCryptographicHash ha2(QCryptographicHash::Sha256);
				ha2.addData("dummy_method:dummy_uri");
				const QString ha2Result = QString::fromLatin1(ha2.result().toHex());

				QCryptographicHash hash(QCryptographicHash::Sha256);
				const QString hashString = ha1Result +
					":" + nonce +
					":" + QString::number(nc) +
					":" + authenticationObject["cnonce"].toString() +
					":auth" +
					":" + ha2Result;

				hash.addData(hashString.toLatin1());
				authenticationObject["response"] = QString::fromLatin1(hash.result().toHex());

				QJsonObject requestObjectWithAuth = requestObject;
				requestObjectWithAuth["auth"] = authenticationObject;

				QNetworkReply* newReply = m_networkAccessManager->post(request, QJsonDocument(requestObjectWithAuth).toJson());
				connect(newReply, &QNetworkReply::finished, this, [newReply, parseAnswer]()
				{
					if (newReply->error() != QNetworkReply::NoError)
					{
						qDebug() << "Error after auth" << newReply->errorString();
						newReply->deleteLater();
						return;
					}

					parseAnswer(newReply->readAll());
					newReply->deleteLater();
				});
			}

			reply->deleteLater();
			return;
		}

		if (reply->error() != QNetworkReply::NoError)
		{
			qDebug() << reply->errorString();
			reply->deleteLater();
			return;
		}

		parseAnswer(reply->readAll());
		reply->deleteLater();
	});
}

void LightPresets::buildButtons(const QJsonObject& source)
{
	auto buttonArray = source["buttons"].toArray();
	const int startXOffset = width();
	constexpr int BUTTON_WIDTH = 100;
	constexpr int BUTTONS_PER_COLUMN = 2;

	const int numberOfColumns = std::ceil(buttonArray.size() / BUTTONS_PER_COLUMN);
	const int newWidth = width() + (numberOfColumns * BUTTON_WIDTH);
	resize(newWidth, height());
	setMinimumWidth(newWidth);
	setMaximumWidth(newWidth);

	for (int i=0; i < numberOfColumns; i++)
	{
		QWidget *base = new QWidget(this); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
		base->setGeometry(QRect(startXOffset + (i * BUTTON_WIDTH), 0, BUTTON_WIDTH, height()));

		QVBoxLayout *layout = new QVBoxLayout(); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
		base->setLayout(layout);
		layout->setSizeConstraint(QLayout::SetFixedSize);
		layout->setSpacing(0);
		layout->setContentsMargins( 0, 0, 0, 0);

		for (int j=0; j < BUTTONS_PER_COLUMN; j++)
		{
			int buttonIndex = (i * BUTTONS_PER_COLUMN) + j;
			if (buttonIndex >= buttonArray.size())
			{
				break;
			}

			auto buttonObject = buttonArray[buttonIndex].toObject();
			auto *label = new QLabel(buttonObject["name"].toString(), this); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
			label->setMaximumHeight(16);
			layout->addWidget(label);

			QWidget *buttonBase = new QWidget(this); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
			layout->addWidget(buttonBase);
			QHBoxLayout *buttonLayout = new QHBoxLayout(); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
			buttonBase->setLayout(buttonLayout);
			buttonLayout->setSpacing(0);
			buttonLayout->setContentsMargins( 0, 0, 0, 0);

			auto *buttonOff = new QPushButton(tr("Off"), buttonBase); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
			buttonOff->setMaximumWidth(40);
			buttonOff->setMinimumWidth(40);
			buttonOff->setMaximumHeight(16);
			buttonLayout->addWidget(buttonOff);
			buttonLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

			auto *buttonOn = new QPushButton(tr("On"), buttonBase); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
			buttonOn->setMaximumWidth(40);
			buttonOn->setMinimumWidth(40);
			buttonOn->setMaximumHeight(16);
			buttonLayout->addWidget(buttonOn);

			setupButtonCommunicationShelly(buttonObject, buttonOn, buttonOff);
		}
	}
}
