#include <QOAuth2AuthorizationCodeFlow>
#include <QDesktopServices>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QOAuthHttpServerReplyHandler>
#include <QNetworkReply>
#include <QSettings>
#include <QMessageBox>
#include "livestream.h"

#include <qguiapplication.h>

Livestream::Livestream(const QJsonObject& configurationObject, QObject *parent)
	:QObject{parent}
	,m_oauth2(new QOAuth2AuthorizationCodeFlow(this))
{
	m_atemAddress = configurationObject["atemAddress"].toString();
	m_oauth2->setScope("https://www.googleapis.com/auth/youtube");
	connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,&QDesktopServices::openUrl);

	QFile file("client_secret.json");
	if (file.open(QIODevice::ReadOnly) == false)
	{
		liveStreamError("Could not open client_secret.json");
		return;
	}
	const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
	file.close();
	const auto object = document.object();
	const auto settingsObject = object["installed"].toObject();
	const QUrl authUri(settingsObject["auth_uri"].toString());
	const auto clientId = settingsObject["client_id"].toString();
	const QUrl tokenUri(settingsObject["token_uri"].toString());
	const auto clientSecret(settingsObject["client_secret"].toString());
	const auto redirectUris = settingsObject["redirect_uris"].toArray();
	const QUrl redirectUri(redirectUris[0].toString()); // Get the first URI
	const auto port = static_cast<quint16>(redirectUri.port()); // Get the port

	m_oauth2->setAuthorizationUrl(authUri);
	m_oauth2->setClientIdentifier(clientId);
	m_oauth2->setAccessTokenUrl(tokenUri);
	m_oauth2->setClientIdentifierSharedKey(clientSecret);

	auto replyHandler = new QOAuthHttpServerReplyHandler(5476, this); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
	m_oauth2->setReplyHandler(replyHandler);

	connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::requestFailed, this, [this](const QAbstractOAuth::Error error)
	{
		liveStreamError("OAuth2 request failed: " + QString::number(static_cast<int>(error)));
	});
}

void Livestream::startStream()
{
	disconnect(m_oauth2, &QOAuth2AuthorizationCodeFlow::granted, nullptr, nullptr);
	connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &Livestream::youtubeAccessGranted);

	QSettings settings;
	const auto token = settings.value("youtubeToken").toString();
	const auto refreshToken = settings.value("youtubeRefreshToken").toString();
	if (token.isEmpty() == false && refreshToken.isEmpty() == false)
	{
		m_oauth2->setToken(token);
		m_oauth2->setRefreshToken(refreshToken);
		m_oauth2->refreshAccessToken();
	}
	else
	{
		m_oauth2->grant();
	}
}

void Livestream::stopStream()
{
	auto tcpSocket = new QTcpSocket(this); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
	connect(tcpSocket, &QTcpSocket::connected, this, [tcpSocket]
	{
		qDebug() << "Connection established";
		tcpSocket->write("stream stop\n");
		tcpSocket->flush();
		tcpSocket->deleteLater();
	});
	connect(tcpSocket, &QTcpSocket::errorOccurred, this, [tcpSocket](QAbstractSocket::SocketError socketError)
	{
		liveStreamError("Error connecting to ATEM Mini Pro: " + tcpSocket->errorString());
		tcpSocket->deleteLater();
	});
	tcpSocket->connectToHost(m_atemAddress, 9993);
}

void Livestream::youtubeAccessGranted()
{
	QSettings settings;
	settings.setValue("youtubeToken", m_oauth2->token());
	settings.setValue("youtubeRefreshToken", m_oauth2->refreshToken());

	auto reply = m_oauth2->get(QUrl("https://www.googleapis.com/youtube/v3/liveBroadcasts?part=snippet,contentDetails&broadcastStatus=upcoming"));
	connect(reply, &QNetworkReply::finished, this, [this, reply]
	{
		if (reply->error() != QNetworkReply::NoError)
		{
			liveStreamError("Error retrieving broadcasts: " + reply->errorString());
			reply->deleteLater();
			return;
		}

		youtubeListBroadcastsFinished(reply->readAll());
		reply->deleteLater();
	});
}

void Livestream::youtubeListBroadcastsFinished(const QByteArray& data)
{
	QJsonDocument document = QJsonDocument::fromJson(data);
	QJsonObject object = document.object();
	QJsonArray items = object["items"].toArray();

	QJsonObject mostRecentUpcomingBroadcast;

	for (const QJsonValueRef& itemValue : items)
	{
		QJsonObject item = itemValue.toObject();
		if (item["snippet"].toObject().contains("scheduledStartTime") == true)
		{
			QJsonObject snippet = item["snippet"].toObject();
			QDateTime scheduledStartTime = QDateTime::fromString(snippet["scheduledStartTime"].toString(), Qt::ISODate);
			if (mostRecentUpcomingBroadcast.isEmpty() == true)
			{
				mostRecentUpcomingBroadcast = item;
			}
			else
			{
				QJsonObject mostRecentSnippet = mostRecentUpcomingBroadcast["snippet"].toObject();
				QDateTime mostRecentScheduledStartTime = QDateTime::fromString(mostRecentSnippet["scheduledStartTime"].toString(), Qt::ISODate);
				if (qAbs(QDateTime::currentDateTime().msecsTo(scheduledStartTime)) < qAbs(QDateTime::currentDateTime().msecsTo(mostRecentScheduledStartTime)))
				{
					mostRecentUpcomingBroadcast = item;
				}
			}
		}
	}

	if (mostRecentUpcomingBroadcast.isEmpty() == true)
	{
		liveStreamError("No upcoming broadcasts found.");
		return;
	}

	QString boundStreamId = mostRecentUpcomingBroadcast["contentDetails"].toObject()["boundStreamId"].toString();
	auto reply = m_oauth2->get(QUrl("https://www.googleapis.com/youtube/v3/liveStreams?part=snippet,cdn&id=" + boundStreamId));
	connect(reply, &QNetworkReply::finished, this, [this, reply]
	{
		if (reply->error() != QNetworkReply::NoError)
		{
			liveStreamError("Error retrieving broadcasts: " + reply->errorString());
			reply->deleteLater();
			return;
		}

		youtubeGetStreamFinished(reply->readAll());
		reply->deleteLater();
	});
}

void Livestream::youtubeGetStreamFinished(const QByteArray& data)
{
	QJsonDocument document = QJsonDocument::fromJson(data);
	QJsonObject object = document.object();
	QJsonArray items = object["items"].toArray();
	if (items.isEmpty() == true)
	{
		liveStreamError("No live stream information found.");
		return;
	}

	QString liveStreamUrl = items[0].toObject()["cdn"].toObject()["ingestionInfo"].toObject()["ingestionAddress"].toString();
	QString streamName = items[0].toObject()["cdn"].toObject()["ingestionInfo"].toObject()["streamName"].toString();

	qDebug() << "Live Stream URL:" << liveStreamUrl;
	qDebug() << "Stream Name/Key:" << streamName;

	auto tcpSocket = new QTcpSocket(this); //NOLINT (cppcoreguidelines-owning-memory) Memory is managed by Qt
	connect(tcpSocket, &QTcpSocket::connected, this, [tcpSocket, liveStreamUrl, streamName]
	{
		qDebug() << "Connection established";
		tcpSocket->write(QString("stream start: url: %1 key: %2\n").arg(liveStreamUrl, streamName).toLatin1());
		tcpSocket->flush();
		tcpSocket->deleteLater();
	});
	connect(tcpSocket, &QTcpSocket::errorOccurred, this, [tcpSocket](QAbstractSocket::SocketError socketError)
	{
		liveStreamError("Error connecting to ATEM Mini Pro: " + tcpSocket->errorString());
		tcpSocket->deleteLater();
	});
	tcpSocket->connectToHost(m_atemAddress, 9993);
}

void Livestream::liveStreamError(const QString& errorString)
{
	qCritical() << errorString;
	QMessageBox::critical(nullptr, tr("Livestream Error"), errorString);
}
