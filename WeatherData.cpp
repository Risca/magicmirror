#include "WeatherData.h"

WeatherData::WeatherData(QObject *parent) : QObject(parent)
{
    m_forecast = new QNetworkAccessManager(this);
    m_current = new QNetworkAccessManager(this);

    connect(m_forecast, SIGNAL(finished(QNetworkReply*)), this, SLOT(forecastReplyFinished(QNetworkReply*)));
    connect(m_current, SIGNAL(finished(QNetworkReply*)), this, SLOT(currentReplyFinished(QNetworkReply*)));
}

WeatherData::~WeatherData()
{
}

void WeatherData::addTownId(const QString& townid)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_townID = townid;
}

void WeatherData::addAppID(QString a)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_appID = a;
}

void WeatherData::processCurrentWeather()
{
    QUrl u("http://api.openweathermap.org/data/2.5/weather");
    QUrlQuery query;

    qDebug() << __PRETTY_FUNCTION__;
    query.addQueryItem("appid", m_appID);
    query.addQueryItem("id", m_townID);
    query.addQueryItem("units", "metric");
    u.setQuery(query);


    m_current->get(QNetworkRequest(u));
}

void WeatherData::processForecast()
{
    QUrl u("http://api.openweathermap.org/data/2.5/forecast");
    QUrlQuery query;

    qDebug() << __PRETTY_FUNCTION__;
    query.addQueryItem("appid", m_appID);
    query.addQueryItem("id", m_townID);
    query.addQueryItem("units", "metric");
    query.addQueryItem("cnt", "5");
    u.setQuery(query);

    m_forecast->get(QNetworkRequest(u));
}

void WeatherData::currentReplyFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << reply->errorString();
    }
    else {
        QJsonDocument jdoc = QJsonDocument::fromJson(reply->readAll());
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
    reply->deleteLater();
}

void WeatherData::forecastReplyFinished(QNetworkReply *reply)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << reply->errorString();
    }
    else {
        QJsonDocument jdoc = QJsonDocument::fromJson(reply->readAll());
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
    reply->deleteLater();
}
