#ifndef LIVESTREAM_H
#define LIVESTREAM_H

#include <QObject>
#include <QJsonObject>

class QOAuth2AuthorizationCodeFlow;

class Livestream : public QObject
{
	Q_OBJECT
public:
	explicit Livestream(const QJsonObject& configurationObject, QObject *parent = nullptr);

public slots:
	void startStream();
	void stopStream();

private slots:
	void youtubeAccessGranted();
	void youtubeListBroadcastsFinished(const QByteArray &data);
	void youtubeGetStreamFinished(const QByteArray &data);

private:
	static void liveStreamError(const QString &errorString);

	QString m_atemAddress;
	QOAuth2AuthorizationCodeFlow *m_oauth2 = nullptr;
};

#endif // LIVESTREAM_H
