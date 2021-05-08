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

const QString BASE_URL = "https://www.googleapis.com/calendar/v3/";
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

QColor colorFromHexString(const QString &string)
{
    QColor c;
    c.setNamedColor(string);
    return c;
}

QMap<int, QColor> toColorMap(const QJsonObject &list)
{
    QMap<int, QColor> map;
    for (QJsonObject::const_iterator color = list.constBegin();
         color != list.constEnd(); color ++)
    {
#if QT_VERSION >= 0x050a00
        const QString &hex = color.value()["background"].toString();
#else
        const QString &hex = color.value().toObject()["background"].toString();
#endif
        map[color.key().toInt()] = colorFromHexString(hex);
    }
    return map;
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
    m_linkingInProgress(false),
    m_ids(calendars),
    m_colorRequest(-1),
    m_calendarInfoRequest(-1)
{
    m_o2->setParent(this);

    connect(o2, &O2::linkedChanged, this, &GoogleCalendarSource::onLinkedChanged);
    connect(o2, &O2::linkingFailed, this, &GoogleCalendarSource::onLinkingFailed);
    connect(o2, &O2::linkingSucceeded, this, &GoogleCalendarSource::onLinkingSucceeded);
    connect(o2, &O2::showVerificationUriAndCode, this, &GoogleCalendarSource::onVerificationCodeAndUrl);
    connect(o2, &O2::refreshFinished, this, &GoogleCalendarSource::onRefreshFinished);

    // configure requestor
    m_requestor->setAddAccessTokenInQuery(false);
    m_requestor->setAccessTokenInAuthenticationHTTPHeaderFormat("Bearer %1");

#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
    void (O2Requestor::*requestorFinished)(int,QNetworkReply::NetworkError,QByteArray) =
            &O2Requestor::finished;
    connect(m_requestor, requestorFinished, this, &GoogleCalendarSource::onFinished);
#else
    connect(m_requestor, QOverload<int,QNetworkReply::NetworkError, QByteArray>::of(&O2Requestor::finished),
        this, &GoogleCalendarSource::onFinished);
#endif

    m_retryTimer.setTimerType(Qt::VeryCoarseTimer);
    m_retryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, &QTimer::timeout, this, &GoogleCalendarSource::sync);

    m_refreshTimer.setTimerType(Qt::VeryCoarseTimer);
    m_refreshTimer.setSingleShot(true);
    connect(&m_refreshTimer, &QTimer::timeout, m_o2, &O2::refresh);
}

GoogleCalendarSource::~GoogleCalendarSource()
{
    delete m_o2;
}

void GoogleCalendarSource::sync()
{
    qDebug() << __PRETTY_FUNCTION__;

    m_retryTimer.stop();

    if (!m_linkingInProgress) {
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
    m_linkingInProgress = false;
    m_refreshTimer.stop();
    m_retryTimer.start();
}

void GoogleCalendarSource::onLinkingSucceeded()
{
    m_linkingInProgress = false;
    if (isAccessTokenValid()) {
        getColors();
    }
}

void GoogleCalendarSource::onVerificationCodeAndUrl(const QUrl &url, const QString &code)
{
    qDebug() << __PRETTY_FUNCTION__ << url << code;

    utils::QrCodePopup *popup = new utils::QrCodePopup(QSize(300, 300), url.toString());

    popup->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_o2, &O2::closeBrowser, popup, &QWidget::close);
    connect(m_o2, &O2::linkingFailed, popup, &QWidget::close);

    popup->show();
    popup->showMessage(QString("%1\n%3").arg("Google calendar code:").arg(code));
}

void GoogleCalendarSource::onFinished(int id, QNetworkReply::NetworkError error, const QByteArray &data)
{
    if (id != m_colorRequest && id != m_calendarInfoRequest && !m_currentRequest.contains(id)) {
        qWarning() << __PRETTY_FUNCTION__ << "invalid request id:" << id << m_colorRequest;
        return;
    }

    const QString &calendar = m_currentRequest.value(id);
    qDebug() << __PRETTY_FUNCTION__ << id << (id == m_colorRequest ? "colors" :
                                             (id == m_calendarInfoRequest ? "calendar info" :
                                             calendar));

    if (error != QNetworkReply::NoError) {
        qWarning() << __PRETTY_FUNCTION__ << "error:" << error;
        m_retryTimer.start();
    }
    else {
        QJsonDocument jdoc = QJsonDocument::fromJson(data);

        if (id == m_colorRequest) {
            m_colorRequest = -1;

            m_calendarColorsByColorId = toColorMap(jdoc.object()["calendar"].toObject());
            m_eventColors = toColorMap(jdoc.object()["event"].toObject());

            getCalendarInfo();
        }
        else if (id == m_calendarInfoRequest) {
            m_calendarInfoRequest = -1;

            m_calendarColorsByCalendarId.clear();
            foreach (const QJsonValue &i, jdoc.object()["items"].toArray()) {
                QJsonObject calendar = i.toObject();
                const QString calendarId = calendar["id"].toString();
                if (m_ids.contains(calendarId)) {
                    const QString hex = calendar["backgroundColor"].toString();
                    m_calendarColorsByCalendarId[calendarId] = colorFromHexString(hex);
                }
            }
            // Got our info, let's get the first calendar
            getEvents(m_ids.first());
        }
        else {
            addEvents(jdoc, calendar);
        }
    }

    if (!calendar.isNull()) {
        const int nextCalendarIndex = m_ids.indexOf(calendar) + 1;
        if (nextCalendarIndex == m_ids.size()) {
            // Fetched all events, time to publish
            std::sort(m_events.begin(), m_events.end());
            emit finished(m_events);
            m_events.clear();
        }
        else {
            // Fetch next calendar's events
            getEvents(m_ids[nextCalendarIndex]);
        }
    }
}

void GoogleCalendarSource::onRefreshFinished(QNetworkReply::NetworkError error)
{
    qDebug() << __PRETTY_FUNCTION__ << error;
    switch (error) {
    case QNetworkReply::NoError:
        // this restarts the refresh timer
        isAccessTokenValid();
        break;
    case QNetworkReply::InternalServerError:
    case QNetworkReply::ServiceUnavailableError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::UnknownNetworkError:
    case QNetworkReply::UnknownProxyError:
    case QNetworkReply::UnknownServerError:
    case QNetworkReply::ProtocolFailure:
        // try again later
        m_retryTimer.start();
        break;
    default:
        // refresh failed, try linking instead
        m_o2->link();
    }
}

void GoogleCalendarSource::getColors()
{
    QUrl url = BASE_URL + "colors";
    QNetworkRequest req(url);
    m_colorRequest = m_requestor->get(req);
}

void GoogleCalendarSource::getCalendarInfo()
{
    QUrl url = BASE_URL + "users/me/calendarList";
    QNetworkRequest req(url);
    m_calendarInfoRequest = m_requestor->get(req);
}

void GoogleCalendarSource::getEvents(const QString &calendar)
{
    QUrl url = BASE_URL + "calendars/" + calendar + "/events";
    QNetworkRequest req(url);
    int id = m_requestor->get(req);
    m_currentRequest[id] = calendar;
}

bool GoogleCalendarSource::isAccessTokenValid()
{
    int expires = m_o2->expires()*1000 - QDateTime::currentMSecsSinceEpoch();
    if (expires <= 0) {
        qDebug() << __PRETTY_FUNCTION__ << "already expired - refreshing";
        if (!m_o2->refreshToken().isEmpty()) {
            m_o2->refresh();
        }
        else {
            qDebug() << __PRETTY_FUNCTION__ << "no refresh token - linking instead";
            m_o2->link();
        }
        return false;
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << "expires in" << expires/1000 << "seconds";
        m_refreshTimer.start(expires);
    }
    return true;
}

void GoogleCalendarSource::addEvents(const QJsonDocument &jdoc, const QString &calendarId)
{
    QJsonArray items = jdoc.object()["items"].toArray();

    const QDate today = QDate::currentDate();
    foreach (const QJsonValue &i, items) {
        QJsonObject event = i.toObject();
        calendar::Event e;

        e.stop = toDate(event["end"]);
        if (e.stop >= today) {
            e.start = toDate(event["start"]);
            // Google calendar sets end date to the day after last, unless
            // it's a same day event.
            if (e.start != e.stop)
                e.stop = e.stop.addDays(-1);
            e.summary = event["summary"].toString();
            e.color = getEventColor(event, calendarId);
            m_events.push_back(e);
        }
    }
}

QColor GoogleCalendarSource::getEventColor(const QJsonObject &event, const QString &calendarId)
{
    // Check if event has specific color
    bool ok;
    int colorId = event["colorId"].toString().toInt(&ok);
    if (ok && m_eventColors.contains(colorId)) {
        return m_eventColors[colorId];
    }

    // fallback
    return m_calendarColorsByCalendarId.value(calendarId, Qt::gray);
}

} // namespace calendar
