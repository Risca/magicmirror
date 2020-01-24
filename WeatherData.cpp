#include "WeatherData.h"

#include "settingsfactory.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QUrlQuery>

#define DEFAULT_RETRY_TIMEOUT (1000 * 30)
#define DEFAULT_WEATHER_URL "https://api.openweathermap.org/data/2.5/weather"

WeatherData::WeatherData(const QString &appId, const QString &townId, QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    QObject(parent), m_net(net), m_forecast(0), m_current(0), m_appID(appId), m_townID(townId)
{
    m_forecastRetryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    m_forecastRetryTimer.setSingleShot(true);
    connect(&m_forecastRetryTimer, SIGNAL(timeout()), this, SLOT(processForecast()));

    m_currentRetryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    m_currentRetryTimer.setSingleShot(true);
    connect(&m_currentRetryTimer, SIGNAL(timeout()), this, SLOT(processCurrentWeather()));
}

bool WeatherData::Create(WeatherData *&weatherData, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Weather");
    const QString appId = settings->value("appid").toString();
    const QString townId = settings->value("townid").toString();
    if (appId.isEmpty() || townId.isEmpty()) {
        return false;
    }
    weatherData = new WeatherData(appId, townId, net, parent);
    return true;
}

WeatherData::~WeatherData()
{
}

void WeatherData::processCurrentWeather()
{
    if (m_current) {
        qWarning() << __PRETTY_FUNCTION__ << ": current weather update request already in process - aborting";
        return;
    }

    m_currentRetryTimer.stop();

    QUrl u(DEFAULT_WEATHER_URL);
    QUrlQuery query;

    qDebug() << __PRETTY_FUNCTION__;
    query.addQueryItem("appid", m_appID);
    query.addQueryItem("id", m_townID);
    query.addQueryItem("units", "metric");
    u.setQuery(query);

    m_current = m_net->get(QNetworkRequest(u));
    connect(m_current, SIGNAL(finished()), this, SLOT(currentReplyFinished()));
}

void WeatherData::processForecast()
{
    if (m_forecast) {
        qWarning() << __PRETTY_FUNCTION__ << ": weather forecast request already in process - aborting";
        return;
    }

    m_forecastRetryTimer.stop();

    QUrl u(DEFAULT_WEATHER_URL);
    QUrlQuery query;

    qDebug() << __PRETTY_FUNCTION__;
    query.addQueryItem("appid", m_appID);
    query.addQueryItem("id", m_townID);
    query.addQueryItem("units", "metric");
    query.addQueryItem("cnt", "5");
    u.setQuery(query);

    m_forecast = m_net->get(QNetworkRequest(u));
    connect(m_forecast, SIGNAL(finished()), this, SLOT(forecastReplyFinished()));
}

void WeatherData::currentReplyFinished()
{
    if (m_current->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_current->errorString();
        m_currentRetryTimer.start();
    }
    else {
        QJsonDocument jdoc = QJsonDocument::fromJson(m_current->readAll());
        QJsonObject jobj = jdoc.object();
        QJsonObject main = jobj["main"].toObject();
        emit temperature(main["temp"].toDouble());
        emit humidity(main["humidity"].toDouble());
        QJsonObject wind = jobj["wind"].toObject();
        emit windSpeed(wind["speed"].toDouble());

        QJsonObject sys = jobj["sys"].toObject();
        emit sunrise(static_cast<qint64>(sys["sunrise"].toInt()));
        emit sunset(static_cast<qint64>(sys["sunset"].toInt()));
        QJsonArray weather = jobj["weather"].toArray();
        for (int i = 0; i < weather.size(); ++i) {
            QJsonObject obj = weather[i].toObject();
            emit skyConditions(obj["main"].toString());
            emit currentIcon(obj["icon"].toString());
        }
    }
    emit finished();
    m_current->deleteLater();
    m_current = 0;
}

void WeatherData::forecastReplyFinished()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (m_forecast->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_forecast->errorString();
        m_forecastRetryTimer.start();
    }
    else {
        QJsonDocument jdoc = QJsonDocument::fromJson(m_forecast->readAll());
        QJsonObject jobj = jdoc.object();
        QJsonArray entries = jobj["list"].toArray();
        QJsonArray weather = jobj["weather"].toArray();
        emit forecastEntryCount(entries.count());
        qDebug() << __PRETTY_FUNCTION__ << ": sending" << entries.count() << "entries to the mirror";
        for (int i = 0; i < entries.size(); i++) {
            emit forecastEntry(entries[i].toObject());
        }
    }
    emit finished();
    m_forecast->deleteLater();
    m_forecast = 0;
}
