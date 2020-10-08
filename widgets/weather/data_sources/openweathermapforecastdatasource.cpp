#include "openweathermapforecastdatasource.h"

#include "utils/iconcache.h"
#include "utils/sensordata.h"

#include <QtGlobal>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace weather {

namespace {

QUrl IconUrl(const QString &icon)
{
    return QUrl("https://openweathermap.org/img/wn/" + icon + ".png");
}

} // anonymous namespace

#define DEFAULT_RETRY_TIMEOUT   (30 * 1000)
#define DEFAULT_UPDATE_INTERVAL (4 * 60 * 60 * 1000)
#define DEFAULT_FORECAST_URL "https://api.openweathermap.org/data/2.5/forecast"

OpenWeatherMapForecastDataSource::OpenWeatherMapForecastDataSource(
        const QString &appId,
        const QString &townId,
        QSharedPointer<QNetworkAccessManager> net,
        QObject *parent) :
    IForecastDataSource(parent),
    m_net(net),
    m_reply(0),
    m_appID(appId),
    m_townID(townId),
    m_iconCache(0)
{
    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setSingleShot(true);
    m_timer.setInterval(DEFAULT_RETRY_TIMEOUT);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(requestForecast()));

    m_iconCache = new IconCache(net, this);
    connect(m_iconCache, SIGNAL(iconDownloaded(const QString&)), this, SLOT(updateIcon(const QString&)));

    requestForecast();
}

OpenWeatherMapForecastDataSource::~OpenWeatherMapForecastDataSource()
{
    delete m_reply;
    delete m_iconCache;
}

const QList<utils::SensorData> &OpenWeatherMapForecastDataSource::forecast() const
{
    return m_currentForecast;
}

void OpenWeatherMapForecastDataSource::requestForecast()
{
    if (m_reply) {
        qWarning() << __PRETTY_FUNCTION__ << ": weather forecast request already in process - aborting";
        return;
    }

    QUrl u(DEFAULT_FORECAST_URL);
    QUrlQuery query;

    qDebug() << __PRETTY_FUNCTION__;

    query.addQueryItem("appid", m_appID);
    query.addQueryItem("id", m_townID);
    query.addQueryItem("units", "metric");
    query.addQueryItem("cnt", "10");
    u.setQuery(query);

    m_reply = m_net->get(QNetworkRequest(u));
    connect(m_reply, SIGNAL(finished()), this, SLOT(forecastRequestFinished()));
}

void OpenWeatherMapForecastDataSource::forecastRequestFinished()
{
    if (m_reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_reply->errorString();
        m_timer.setInterval(DEFAULT_RETRY_TIMEOUT);
    }
    else {
        const QDate today = QDate::currentDate();
        QList<utils::SensorData> forecast;
        QList<QString> icons;
        QJsonDocument jdoc = QJsonDocument::fromJson(m_reply->readAll());
        QJsonArray entries = jdoc.object()["list"].toArray();

        qDebug() << __PRETTY_FUNCTION__ << ": sending" << entries.count() << "entries to the mirror";

        for (int i = 0; i < entries.count(); i++) {
            QJsonObject jobj = entries[i].toObject();
            double temp_min, temp_max;
            utils::SensorData d;

            qint64 secs = jobj["dt"].toInt();
#if (QT_VERSION < QT_VERSION_CHECK(5, 8, 0))
            d.timestamp = QDateTime::fromMSecsSinceEpoch(secs * 1000);
#else
            d.timestamp = QDateTime::fromSecsSinceEpoch(secs);
#endif
            if (d.timestamp.date() < today) {
                qWarning() << "Ignoring forecast in the past:" << d.timestamp;
                continue;
            }

            QJsonObject main = jobj["main"].toObject();
            d.values[utils::HUMIDITY] = main["humidity"].toDouble();
            temp_min = main["temp_min"].toDouble();
            temp_max = main["temp_max"].toDouble();
            if (temp_max - temp_min <= 0.05) {
                d.values[utils::TEMPERATURE] = temp_max;
            }
            else {
                d.values[utils::TEMPERATURE_LOW] = temp_min;
                d.values[utils::TEMPERATURE_HIGH] = temp_max;
            }
            d.values[utils::WIND_SPEED] = jobj["wind"].toObject()["speed"].toDouble();
            if (jobj.contains("rain")) {
                d.values[utils::PRECIPITATION] = jobj["rain"].toObject()["3h"].toDouble();
            }

            const QString icon = jobj["weather"].toArray().first().toObject()["icon"].toString();
            if (m_iconCache->exists(icon)) {
                m_iconCache->get(icon, d.icon);
            }
            else {
                m_iconCache->download(icon, IconUrl(icon));
            }
            icons.push_back(icon);
            forecast.push_back(d);
        }
        m_forecastIcons = icons;
        m_currentForecast = forecast;
        emit newForecastAvailable();
        m_timer.setInterval(DEFAULT_UPDATE_INTERVAL);
    }

    m_reply->deleteLater();
    m_reply = 0;

    m_timer.start();
}

void OpenWeatherMapForecastDataSource::updateIcon(const QString &icon)
{
    bool updated = false;
    for (int i = 0; i < m_forecastIcons.count(); ++i) {
        if (icon == m_forecastIcons[i]) {
            m_iconCache->get(icon, m_currentForecast[i].icon);
            updated = true;
        }
    }
    if (updated) {
        emit newForecastAvailable();
    }
}

} // namespace weather
