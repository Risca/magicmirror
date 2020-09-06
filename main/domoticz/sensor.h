#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>
#include <QSharedPointer>
#include <QTimer>
#include <QNetworkRequest>

class QDateTime;
class QNetworkAccessManager;
class QNetworkReply;
class QString;

#define DOMOTICZ_SETTINGS_GROUP                 "Domoticz"
#define DOMOTICZ_INDOOR_TEMP_IDX_SETTINGS_KEY   "indoorTempIdx"

namespace domoticz {

class Sensor : public QObject
{
    Q_OBJECT
public:
    static bool Create(Sensor*& sensor, int idx, QSharedPointer<QNetworkAccessManager> net, QObject* parent = 0);

public slots:
    void update();

signals:
    void valueUpdated(const QString& sensor, const QString& value, const QDateTime& lastUpdated);

protected:
    Q_DISABLE_COPY(Sensor)
    explicit Sensor(const QNetworkRequest& req, QSharedPointer<QNetworkAccessManager> net, QObject* parent);

    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply* m_reply;
    const QNetworkRequest m_request;
    QTimer m_retryTimer;

protected slots:
    void onReplyFinished();

};

} // namespace domoticz

#endif // SENSOR_H
