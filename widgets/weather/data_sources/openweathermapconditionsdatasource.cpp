#include "openweathermapconditionsdatasource.h"

#include "iconcache.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
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
#define DEFAULT_WEATHER_URL "https://api.openweathermap.org/data/2.5/weather"

OpenWeatherMapConditionsDataSource::OpenWeatherMapConditionsDataSource(
        const QString &appId,
        const QString &townId,
        QSharedPointer<QNetworkAccessManager> net,
        QObject *parent) :
    ICurrentConditionsDataSource(parent),
    m_net(net),
    m_reply(0),
    m_appID(appId),
    m_townID(townId),
    m_iconCache(0)
{
    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setSingleShot(true);
    m_timer.setInterval(DEFAULT_RETRY_TIMEOUT);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(requestWeatherConditions()));

    m_iconCache = new IconCache(net, this);
    connect(m_iconCache, SIGNAL(iconDownloaded(const QString&)), this, SLOT(iconDownloaded(const QString&)));

    requestWeatherConditions();
}

OpenWeatherMapConditionsDataSource::~OpenWeatherMapConditionsDataSource()
{
    delete m_reply;
    delete m_iconCache;
}

void OpenWeatherMapConditionsDataSource::requestWeatherConditions()
{
    if (m_reply) {
        qWarning() << __PRETTY_FUNCTION__ << ": current weather update request already in process - aborting";
        return;
    }

    m_timer.stop();

    QUrl u(DEFAULT_WEATHER_URL);
    QUrlQuery query;

    qDebug() << __PRETTY_FUNCTION__;
    query.addQueryItem("appid", m_appID);
    query.addQueryItem("id", m_townID);
    query.addQueryItem("units", "metric");
    u.setQuery(query);

    m_reply = m_net->get(QNetworkRequest(u));
    connect(m_reply, SIGNAL(finished()), this, SLOT(requestFinished()));
}

void OpenWeatherMapConditionsDataSource::requestFinished()
{

    if (m_reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_reply->errorString();
        m_timer.start();
    }
    else {
        QJsonDocument jdoc = QJsonDocument::fromJson(m_reply->readAll());
        QJsonObject jobj = jdoc.object();
        QJsonObject main = jobj["main"].toObject();
        emit temperature(main["temp"].toDouble());
        emit humidity(main["humidity"].toDouble());
        QJsonObject wind = jobj["wind"].toObject();
        emit windSpeed(wind["speed"].toDouble());

        QJsonObject sys = jobj["sys"].toObject();
        //emit sunrise(static_cast<qint64>(sys["sunrise"].toInt()));
        //emit sunset(static_cast<qint64>(sys["sunset"].toInt()));
        QJsonObject weather = jobj["weather"].toArray().first().toObject();
        emit skyConditions(weather["description"].toString());

        /* OpenWeatherMap gives us an icon with lots of transparent padding.
         * The icon cache will strip that padding and scale up the icon to its
         * original size. To preserve the icon quality, we request an icon 4x
         * the size (200x200), the icon cache will scale it up, only for us to
         * down-scale it again.
         */
        QString icon = weather["icon"].toString();
        icon += "@4x";
        if (m_iconCache->exists(icon)) {
            QPixmap im;
            m_iconCache->get(icon, im);
            emit image(im.scaledToHeight(100));
        }
        else {
            m_iconCache->download(icon, IconUrl(icon));
        }

        m_timer.setInterval(DEFAULT_UPDATE_INTERVAL);
    }

    m_reply->deleteLater();
    m_reply = 0;

    m_timer.start();
}

void OpenWeatherMapConditionsDataSource::iconDownloaded(const QString &icon)
{
    QPixmap im;
    if (m_iconCache->get(icon, im)) {
        emit image(im.scaledToHeight(100));
    }
}

} // namespace weather
