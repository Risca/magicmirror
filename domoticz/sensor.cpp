#include "sensor.h"

#include "settingsfactory.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrlQuery>

#define DEFAULT_RETRY_TIMEOUT (1000 * 30)

namespace domoticz {

namespace {

QString GetEncodedUserName(const QSharedPointer<QSettings>& a_Settings)
{
    QByteArray username = a_Settings->value("username").toByteArray();
    return username.toBase64();
}

QString GetEncodedPassword(const QSharedPointer<QSettings>& a_Settings)
{
    QByteArray password = a_Settings->value("password").toByteArray();
    return password.toBase64();
}

typedef QPair<QString, QString> QueryItem;
typedef QList<QueryItem> QueryList;

} // anonymous namespace

bool Sensor::Create(Sensor *&sensor, int idx, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create(DOMOTICZ_SETTINGS_GROUP);
    if (settings) {
        const QString host = settings->value("hostname").toString();
        const QString username = GetEncodedUserName(settings);
        const QString password = GetEncodedPassword(settings);
        if (!host.isEmpty() && !username.isEmpty() && !password.isEmpty()) {
            // We got the minimum of what we need, let's build our URL
            QueryList queries;
            queries.append(QueryItem("username", username));
            queries.append(QueryItem("password", password));
            queries.append(QueryItem("type", "devices"));
            queries.append(QueryItem("rid", QString::number(idx)));

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

            qDebug() << __PRETTY_FUNCTION__ << ":" << url.toDisplayString();
            sensor = new Sensor(url, net, parent);
            return !!sensor;
        }
    }
    return false;
}

Sensor::Sensor(const QUrl &url, QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    QObject(parent),
    m_net(net),
    m_reply(0),
    m_url(url)
{
    m_retryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, SIGNAL(timeout()), this, SLOT(update()));
}

void Sensor::update()
{
    if (m_reply) {
        qWarning() << __PRETTY_FUNCTION__ << ": request already in progress, abort!";
        return;
    }

    m_retryTimer.stop();

    QNetworkRequest req(m_url);
    m_reply = m_net->get(req);
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
}

void Sensor::onReplyFinished()
{
    if (m_reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_reply->errorString();
        m_retryTimer.start();
    }
    else {
        QJsonDocument jdoc = QJsonDocument::fromJson(m_reply->readAll());
        QJsonObject jobj = jdoc.object();
        foreach (const QJsonValue& sensor, jobj["result"].toArray()) {
            const QJsonObject s = sensor.toObject();
            const QString name = s["Name"].toString();
            const QString data = s["Data"].toString();
            if (!name.isEmpty() && !data.isEmpty()) {
                qDebug() << __PRETTY_FUNCTION__ << ":" << name << "-" << data;
                emit valueUpdated(name, data);
            }
        }
    }

    m_reply->deleteLater();
    m_reply = 0;
}

} // namespace domoticz
