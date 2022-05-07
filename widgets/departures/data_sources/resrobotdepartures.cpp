#include "resrobotdepartures.h"

#include "utils/settingsfactory.h"

#include <algorithm>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QSignalMapper>
#include <QTime>
#include <QUrlQuery>

namespace departure {

namespace {

const char* RESROBOT_URL = "https://api.resrobot.se/v2.1/departureBoard";
const int DEFAULT_RETRY_TIMEOUT = 60 * 1000;

} // anonymous namespace

bool ResrobotDepartures::Create(ISource *&obj, QSharedPointer<QNetworkAccessManager> &net, QObject *parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Resrobot");
    QList<QNetworkRequest> stops;

    QString const apiKey = settings->value("api_key").toString();
    if (!apiKey.isEmpty()) {
        const QString baseUrl = settings->value("url", RESROBOT_URL).toString();
        int numStops = settings->beginReadArray("stops");
        for (int i = 0; i < numStops; ++i) {
            settings->setArrayIndex(i);
            const QString stopId = settings->value("id").toString();
            if (!stopId.isEmpty()) {
                QUrl u(baseUrl);
                QUrlQuery query;

                query.addQueryItem("accessId", apiKey);
                query.addQueryItem("id", stopId);
                const QString direction = settings->value("direction").toString();
                if (!direction.isEmpty()) {
                    query.addQueryItem("direction", direction);
                }
                const QString operators= settings->value("operators").toString();
                if (!operators.isEmpty()) {
                    query.addQueryItem("operators", operators);
                }
                query.addQueryItem("format", "json");
                u.setQuery(query);
                stops.push_back(QNetworkRequest(u));
            }
        }
        settings->endArray();
    }

    if (stops.empty()) {
        return false;
    }

    obj = new ResrobotDepartures(stops, net, parent);
    return true;
}

ResrobotDepartures::ResrobotDepartures(const QList<QNetworkRequest> &stops, QSharedPointer<QNetworkAccessManager> &net, QObject *parent)
    : ISource(parent),
      m_stops(stops),
      m_net(net),
      m_signalMapper(new QSignalMapper(this))
{
    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ResrobotDepartures::requestNewDepartures);
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    connect(&m_signalMapper, static_cast<void(QSignalMapper::*)(QObject*)>(&QSignalMapper::mapped), this, &ResrobotDepartures::requestFinished);
#else
    connect(&m_signalMapper, &QSignalMapper::mappedObject, this, &ResrobotDepartures::requestFinished);
#endif
}

void ResrobotDepartures::requestNewDepartures()
{
    foreach (const QNetworkRequest& request, m_stops) {
        requestDepartures(request);
    }
}

void ResrobotDepartures::requestDepartures(const QNetworkRequest &request)
{
    QNetworkReply *reply = m_net->get(request);
    m_signalMapper.setMapping(reply, reply);
    connect(reply, &QNetworkReply::finished, &m_signalMapper,
#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
            static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
#else
            QOverload<>::of(&QSignalMapper::map));
#endif
}

void ResrobotDepartures::requestFinished(QObject *replyObject)
{
    int minTimeout = DEFAULT_RETRY_TIMEOUT;
    QNetworkReply *reply = dynamic_cast<QNetworkReply*>(replyObject);
    if (!reply) {
        return;
    }

    if (reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << reply->errorString();
    }
    else {
        const QDateTime now = QDateTime::currentDateTime();
        const QJsonDocument jdoc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray entries = jdoc.object()["Departure"].toArray();
        QList<Departure> times;

        for (int i = 0; i < entries.count(); i++) {
            const QJsonObject departure = entries[i].toObject();
            const QDateTime time =
                    QDateTime(QDate::fromString(departure["date"].toString(), Qt::ISODate),
                              QTime::fromString(departure["time"].toString()));
            if (time.isValid()) {
                const int timeLeft = now.msecsTo(time);
                Departure d;
                d.stopID = departure["stopExtId"].toString().toULongLong();
                d.stopName = departure["stop"].toString();
                d.transport = departure["name"].toString();;
                d.time = time;
                qDebug() << __PRETTY_FUNCTION__ << d.stopID << "(" << departure["stopExtId"] << ")" << d.stopName << d.transport << d.time;
                times.push_back(d);
                if (timeLeft < 0) {
                    qWarning() << __PRETTY_FUNCTION__ << "negative time left" << timeLeft;
                }
                else {
                    minTimeout = std::min(minTimeout, timeLeft);
                }
            }
            else {
                qWarning() << "Failed to parse departure time:" << departure;
            }
        }

        if (!times.empty()) {
            emit newDepartureTimes(times);
        }
    }

    const int remainingTime = m_timer.remainingTime();
    switch (remainingTime) {
    case 0: // overdue
        break;
    case -1: // inactive
        m_timer.start(minTimeout);
        break;
    default:
        m_timer.start(std::min(remainingTime, minTimeout));
        break;
    }

    reply->deleteLater();
}

} // namespace departure
