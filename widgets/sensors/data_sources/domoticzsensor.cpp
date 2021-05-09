#include "domoticzsensor.h"

#include "utils/sensordata.h"

#include <QByteArray>
#include <QDebug>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPair>
#include <QSettings>
#include <QSslConfiguration>
#include <QSslError>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

#define LOW_BATTERY_THRESHOLD   (20)
#define DEFAULT_RETRY_TIMEOUT   (1000 * 30)
#define DEFAULT_UPDATE_INTERVAL (4 * 60 * 60 * 1000)

namespace sensors {

namespace {

QByteArray AuthHeader(const QString& username, const QString& password)
{
    const QByteArray credentials = (username + ":" + password).toUtf8();
    return QByteArray("Basic ") + credentials.toBase64();
}

typedef QPair<QString, QString> QueryItem;
typedef QList<QueryItem> QueryList;

} // anonymous namespace

bool DomoticzSensor::Create(ISource *&obj, const QSharedPointer<QSettings> &settings, QSharedPointer<QNetworkAccessManager> &net, QObject *parent)
{
    const QString host = settings->value("hostname").toString();
    const QString username = settings->value("username").toString();
    const QString password = settings->value("password").toString();
    if (!host.isEmpty() && !username.isEmpty() && !password.isEmpty()) {
        // We got the minimum of what we need, let's build our URL
        QueryList queries;
        queries.append(QueryItem("type", "devices"));
        queries.append(QueryItem("filter", settings->value("filter", "all").toString()));
        queries.append(QueryItem("used", "true"));
        queries.append(QueryItem("order", "name"));

        QUrlQuery q;
        q.setQueryItems(queries);

        QUrl url;
        url.setScheme(settings->value("https", true).toBool() ? "https" : "http");
        url.setHost(host);
        if (settings->contains("port")) {
            url.setPort(settings->value("port").toInt());
        }
        url.setPath("/json.htm");
        url.setQuery(q);

        QNetworkRequest req(url);
        req.setRawHeader("Authorization", AuthHeader(username, password));
        if (!settings->value("verify_peer", true).toBool()) {
            qWarning() << __PRETTY_FUNCTION__ << "Not verifying remote peer!";
            QSslConfiguration conf = req.sslConfiguration();
            conf.setPeerVerifyMode(QSslSocket::VerifyNone);
            req.setSslConfiguration(conf);
        }

        obj = new DomoticzSensor(req, net, parent);
        return !!obj;
    }

    return false;
}

DomoticzSensor::DomoticzSensor(const QNetworkRequest &req, QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    ISource(parent), m_net(net), m_request(req), m_reply(0)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << m_request.url().toDisplayString();
    m_retryTimer.setTimerType(Qt::VeryCoarseTimer);
    m_retryTimer.setSingleShot(true);
    m_retryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    connect(&m_retryTimer, &QTimer::timeout, this, &DomoticzSensor::requestSensorData);

    requestSensorData();
}

DomoticzSensor::~DomoticzSensor()
{
    delete m_reply;
}

void DomoticzSensor::requestSensorData()
{
    if (m_reply) {
        qWarning() << __PRETTY_FUNCTION__ << ": request already in progress, abort!";
        return;
    }

    m_retryTimer.stop();

    m_reply = m_net->get(m_request);
    connect(m_reply, &QNetworkReply::finished, this, &DomoticzSensor::downloadFinished);
    connect(m_reply, &QNetworkReply::sslErrors, this, &DomoticzSensor::onSslError);
}

void DomoticzSensor::downloadFinished()
{
    if (m_reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_reply->errorString();
        m_retryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    }
    else {
        QList<utils::SensorData> list;
        QJsonDocument jdoc = QJsonDocument::fromJson(m_reply->readAll());
        QJsonObject jobj = jdoc.object();

        foreach (const QJsonValue& sensor, jobj["result"].toArray()) {
            utils::SensorData d;
            const QJsonObject s = sensor.toObject();
            d.source = s["Name"].toString();
            if (s["Type"].toString() == "Thermostat") {
                bool ok;
                double const value = s["Data"].toString().toDouble(&ok);
                if (ok)
                    d.values[utils::TEMPERATURE] = value;
            }
            if (s.contains("Temp")) {
                d.values[utils::TEMPERATURE] = s["Temp"].toDouble();
            }
            if (s.contains("Humidity")) {
                d.values[utils::HUMIDITY] = s["Humidity"].toDouble();
            }
            if (s["BatteryLevel"].toInt(255) < LOW_BATTERY_THRESHOLD) {
                d.icon = QIcon(":/sensors/icons/low_battery.svg");
            }
            d.timestamp = QDateTime::fromString(s["LastUpdate"].toString(), Qt::ISODate);
            if (!d.source.isEmpty() && !d.values.isEmpty()) {
                qDebug() << __PRETTY_FUNCTION__ << ":" << d.source << "-" << d.timestamp << "-" << d.values.count() << "reading(s)";
                list.append(d);
            }
        }
        emit sensorDataUpdated(list);

        m_retryTimer.setInterval(DEFAULT_UPDATE_INTERVAL);
    }
    m_reply->deleteLater();
    m_reply = 0;

    m_retryTimer.start();
}

void DomoticzSensor::onSslError(const QList<QSslError> &errors)
{
    foreach (const QSslError& e, errors) {
        qWarning() << __PRETTY_FUNCTION__ << e.errorString() << e.certificate();
    }
}

} // namespace sensors
