#pragma once

#include "sensor_isource.h"

#include <QNetworkRequest>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>

class QNetworkReply;
class QNetworkAccessManager;
class QSettings;
class QSslError;

namespace sensors {

class DomoticzSensor : public ISource
{
    Q_OBJECT

public:
    static bool Create(ISource*& obj, const QSharedPointer<QSettings>& settings, QSharedPointer<QNetworkAccessManager>& net, QObject* parent = 0);
    virtual ~DomoticzSensor();

protected slots:
    void requestSensorData();

protected:
    Q_DISABLE_COPY(DomoticzSensor)
    explicit DomoticzSensor(const QNetworkRequest& req, QSharedPointer<QNetworkAccessManager> net, QObject* parent);

    QSharedPointer<QNetworkAccessManager> m_net;
    const QNetworkRequest m_request;
    QNetworkReply* m_reply;
    QTimer m_retryTimer;

protected slots:
    void downloadFinished();
    void onSslError(const QList<QSslError> &errors);
};

} // namespace sensors
