#ifndef LIVESTREAM_H
#define LIVESTREAM_H

#include <QObject>
#include <QJsonObject>

class QOAuth2AuthorizationCodeFlow;
class QTimer;

class Livestream : public QObject
{
	Q_OBJECT
public:
	explicit Livestream(const QJsonObject& configurationObject, QObject *parent = nullptr);

public slots:
	void startStream();
	void stopStream();

signals:
	void status(bool running, int viewers);

private slots:
	void youtubeAccessGranted();
	void youtubeListBroadcastsFinished(const QByteArray &data);
	void youtubeGetStreamFinished(const QByteArray &data);
	void statusTimer();

private:
	static void liveStreamError(const QString &errorString);

	QString m_atemAddress;
	QOAuth2AuthorizationCodeFlow *m_oauth2 = nullptr;
	QTimer *m_statusTimer = nullptr;
};

#endif // LIVESTREAM_H
