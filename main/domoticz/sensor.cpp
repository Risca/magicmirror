#include "sensor.h"

#include "settingsfactory.h"

#include <QDateTime>
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

QByteArray GetAuthHeader(const QString& username, const QString& password)
{
    const QByteArray credentials = (username + ":" + password).toUtf8();
    return QByteArray("Basic ") + credentials.toBase64();
}

typedef QPair<QString, QString> QueryItem;
typedef QList<QueryItem> QueryList;

} // anonymous namespace

bool Sensor::Create(Sensor *&sensor, int idx, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create(DOMOTICZ_SETTINGS_GROUP);
    if (settings) {
        const QString host = settings->value("hostname").toString();
        const QString username = settings->value("username").toString();
        const QString password = settings->value("password").toString();
        if (!host.isEmpty() && !username.isEmpty() && !password.isEmpty()) {
            // We got the minimum of what we need, let's build our URL
            QueryList queries;
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

            QNetworkRequest req(url);
            req.setRawHeader("Authorization", GetAuthHeader(username, password));
            if (!settings->value("verify_peer", true).toBool()) {
                qWarning() << __PRETTY_FUNCTION__ << "Not verifying remote peer!";
                QSslConfiguration conf = req.sslConfiguration();
                conf.setPeerVerifyMode(QSslSocket::VerifyNone);
                req.setSslConfiguration(conf);
            }

            sensor = new Sensor(req, net, parent);
            return !!sensor;
        }
    }
    return false;
}

Sensor::Sensor(const QNetworkRequest &req, QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    QObject(parent),
    m_net(net),
    m_reply(0),
    m_request(req)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << req.url().toDisplayString();
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

    m_reply = m_net->get(m_request);
    connect(m_reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    connect(m_reply, SIGNAL(sslErrors(const QList<QSslError> &)),
            this, SLOT(onSslError(const QList<QSslError> &)));
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
            const QDateTime lastUpdated = QDateTime::fromString(s["LastUpdate"].toString(), Qt::ISODate);
            if (!name.isEmpty() && !data.isEmpty()) {
                qDebug() << __PRETTY_FUNCTION__ << ":" << name << "-" << data << "-" << lastUpdated;
                emit valueUpdated(name, data, lastUpdated);
            }
        }
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void Sensor::onSslError(const QList<QSslError> &errors)
{
    foreach (const QSslError& e, errors) {
        qWarning() << __PRETTY_FUNCTION__ << e.errorString() << e.certificate();
    }
}

} // namespace domoticz
