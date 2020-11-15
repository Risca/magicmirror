#include "googlecalendarsource.h"

#include "event.hpp"
#include "utils/qrcodepopup.h"
#include "utils/settingsfactory.h"

#include "o2/o0settingsstore.h"
#include "o2/o2requestor.h"
#include "o2/o2googledevice.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

#include <algorithm>

namespace calendar {

namespace {

#define DEFAULT_RETRY_TIMEOUT (1000 * 30)

const QString BASE_URL = "https://www.googleapis.com/calendar/v3/calendars/";
const char *SCOPE = "https://www.googleapis.com/auth/calendar";
const char *DEFAULT_STORE_ENCRYPTION_KEY = "T;qVL}Ub*A57hhX=";

QDate toDate(const QJsonValue &v)
{
    QJsonObject value = v.toObject();
    QString date = value["date"].toString();
    if (date.isEmpty()) {
        date = value["dateTime"].toString();
    }
    return QDate::fromString(date, Qt::ISODate);
}

} // anonymous namespace

bool GoogleCalendarSource::Create(ISource *&obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    const QString id = settings->value("client_id").toString();
    const QString secret = settings->value("client_secret").toString();
    const QStringList ids = settings->value("calendar_ids").toStringList();

    if (id.isEmpty() || secret.isEmpty() || ids.empty()) {
        return false;
    }

    O2GoogleDevice *o2 = new O2GoogleDevice();
    if (!o2) {
        return false;
    }

    QString storeKey = DEFAULT_STORE_ENCRYPTION_KEY;
    const QString storePath = settings->value("settings_dir",
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).toString();

    QFile f(settings->value("settings_key_file").toString());
    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&f);
        storeKey = in.readAll();
    }

    QSettings *storeSettings = new QSettings(storePath + QDir::separator() + "google_calendar_store",
                                             QSettings::NativeFormat);
    if (!storeSettings) {
        delete o2;
        return false;
    }

    O0SettingsStore *store = new O0SettingsStore(storeSettings, storeKey, o2);
    if (!store) {
        delete storeSettings;
        delete o2;
        return false;
    }

    store->setGroupKey(settings->group());
    o2->setStore(store);
    o2->setClientId(id);
    o2->setClientSecret(secret);
    o2->setScope(SCOPE);

    obj = new GoogleCalendarSource(o2, ids, net, parent);
    return !!obj;
}

GoogleCalendarSource::GoogleCalendarSource(O2GoogleDevice *o2, const QStringList &calendars, QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    ISource(parent),
    m_net(net),
    m_o2(o2),
    m_requestor(new O2Requestor(m_net.data(), m_o2, this)),
    m_ids(calendars),
    m_requestId(0)
{
    m_o2->setParent(this);

    connect(o2, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
    connect(o2, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
    connect(o2, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
    connect(o2, SIGNAL(showVerificationUriAndCode(QUrl, QString)),
            this, SLOT(onVerificationCodeAndUrl(QUrl, QString)));

    // configure requestor
    m_requestor->setAddAccessTokenInQuery(false);
    m_requestor->setAccessTokenInAuthenticationHTTPHeaderFormat("Bearer %1");

    connect(m_requestor, SIGNAL(finished(int, QNetworkReply::NetworkError, QByteArray)),
        this, SLOT(onFinished(int, QNetworkReply::NetworkError, QByteArray)));

    m_retryTimer.setTimerType(Qt::VeryCoarseTimer);
    m_retryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, SIGNAL(timeout()), this, SLOT(sync()));

    m_refreshTimer.setTimerType(Qt::VeryCoarseTimer);
    m_refreshTimer.setSingleShot(true);
    connect(&m_refreshTimer, SIGNAL(timeout()), m_o2, SLOT(refresh()));
    connect(m_o2, SIGNAL(refreshFinished(QNetworkReply::NetworkError)),
            this, SLOT(onRefreshFinished(QNetworkReply::NetworkError)));
}

GoogleCalendarSource::~GoogleCalendarSource()
{
    delete m_o2;
}

void GoogleCalendarSource::sync()
{
    qDebug() << __PRETTY_FUNCTION__;

    m_retryTimer.stop();

    if (m_o2->linked()) {
        getEvents();
    }
    else {
        m_o2->link();
    }
}

void GoogleCalendarSource::onLinkedChanged()
{
    qDebug() << __PRETTY_FUNCTION__ << "linked:" << m_o2->linked();
    if (!m_o2->linked()) {
        m_refreshTimer.stop();
        m_retryTimer.start();
    }
}

void GoogleCalendarSource::onLinkingFailed()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_refreshTimer.stop();
    m_retryTimer.start();
}

void GoogleCalendarSource::onLinkingSucceeded()
{
    int expires = static_cast<qint64>(m_o2->expires())*1000 - QDateTime::currentMSecsSinceEpoch();
    qDebug() << __PRETTY_FUNCTION__ << "expires in" << expires*1000 << "seconds";
    m_refreshTimer.start(expires);
    getEvents();
}

void GoogleCalendarSource::onVerificationCodeAndUrl(const QUrl &url, const QString &code)
{
    utils::QrCodePopup *popup = new utils::QrCodePopup(QSize(300, 300), url.toString());

    popup->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_o2, SIGNAL(closeBrowser()), popup, SLOT(close()));

    popup->show();
    popup->showMessage(QString("%1\n%3").arg("Google calendar code:").arg(code));
}

void GoogleCalendarSource::onFinished(int id, QNetworkReply::NetworkError error, const QByteArray &data)
{
    if (id != m_requestId) {
        return;
    }

    if (error != QNetworkReply::NoError) {
        qWarning() << __PRETTY_FUNCTION__ << "error:" << error;
        return;
    }

    QJsonDocument jdoc = QJsonDocument::fromJson(data);
    QJsonArray items = jdoc.object()["items"].toArray();

    const QDate today = QDate::currentDate();
    QList<calendar::Event> events;
    foreach (const QJsonValue &i, items) {
        QJsonObject item = i.toObject();
        calendar::Event e;
        // Google calendar sets end date to the day after last
        e.stop = toDate(item["end"]).addDays(-1);
        if (e.stop >= today) {
            e.start = toDate(item["start"]);
            e.summary = item["summary"].toString();
            events.push_back(e);
        }
    }

    std::sort(events.begin(), events.end());

    emit finished(events);
}

void GoogleCalendarSource::onRefreshFinished(QNetworkReply::NetworkError error)
{
    qDebug() << __PRETTY_FUNCTION__ << error;
}

void GoogleCalendarSource::getEvents()
{
    QUrl url = BASE_URL + m_ids.first() + "/events";
    QNetworkRequest req(url);
    m_requestId = m_requestor->get(req);
}

} // namespace calendar
