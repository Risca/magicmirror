#include "icscalendar.h"

#include "settingsfactory.h"

#include <libical/ical.h>

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>

#include <time.h>

#define DEFAULT_RETRY_TIMEOUT (1000 * 30)

namespace {
QString IcalDatePropertyToString(icalproperty* prop)
{
    bool isDate = false;
    const struct icaltimetype t = icalproperty_get_dtstart(prop);
    const icalparameter* p = icalproperty_get_first_parameter(prop, ICAL_VALUE_PARAMETER);
    if (p != 0) {
        const icalvalue_kind kind = icalparameter_value_to_value_kind(icalparameter_get_value(p));
        switch (kind) {
        case ICAL_NO_VALUE:
        case ICAL_DATETIME_VALUE:
            break;
        case ICAL_DATE_VALUE:
            isDate = true;
            break;
        default:
            qWarning() << __PRETTY_FUNCTION__ << ":" << "Unhandled value kind:" << kind;
            return QString();
        }
    }

    const QDateTime start = QDateTime::fromTime_t(icaltime_as_timet(t));
    if (isDate) {
        return start.date().toString(Qt::DefaultLocaleShortDate);
    }
    else {
        return start.toString(Qt::DefaultLocaleShortDate);
    }
}

QString GetEventDate(icalcomponent* c)
{
    QString dateString;
    icalproperty* prop = icalcomponent_get_first_property(c, ICAL_DTSTART_PROPERTY);
    if (prop != 0) {
        dateString += IcalDatePropertyToString(prop);
        prop = icalcomponent_get_first_property(c, ICAL_DTEND_PROPERTY);
        if (prop != 0) {
            dateString += " - ";
            dateString += IcalDatePropertyToString(prop);
        }
    }
    return dateString;
}

QString GetSummary(icalcomponent* c)
{
    return icalcomponent_get_summary(c);
}

} // anonymous namespace

bool IcsCalendar::Create(CalendarInterface*& cal, QSharedPointer<QNetworkAccessManager> net, QObject* parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Calendar");
    if (settings && settings->value("type").toString() == "ics") {
        const QUrl url = settings->value("url").toUrl();
        if (url.isValid()) {
            cal = new IcsCalendar(url, net, parent);
            return !!cal;
        }
    }
    return false;
}

IcsCalendar::IcsCalendar(const QUrl &url, QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    CalendarInterface(parent),
    m_net(net),
    m_reply(0),
    m_url(url)
{
    m_retryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, SIGNAL(timeout()), this, SLOT(sync()));
}

IcsCalendar::~IcsCalendar()
{
    // empty
}

void IcsCalendar::sync()
{
    if (m_reply) {
        qWarning() << __PRETTY_FUNCTION__ << ": request already in progress, abort!";
        return;
    }

    m_retryTimer.stop();

    QNetworkRequest req(m_url);
    m_reply = m_net->get(req);
    connect(m_reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

void IcsCalendar::downloadFinished()
{
    if (m_reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_reply->errorString();
        m_retryTimer.start();
    }
    else {
        icalcomponent* comp = icalparser_parse_string(m_reply->readAll().data());
        if (comp != 0) {
            for (icalcomponent* c = icalcomponent_get_first_component(comp, ICAL_VEVENT_COMPONENT);
                 c != 0;
                 c = icalcomponent_get_next_component(comp, ICAL_VEVENT_COMPONENT))
            {
                const QString event(GetEventDate(c) + QString(" : ") + GetSummary(c));
                emit newEvent(event);
            }
        }
        icalcomponent_free(comp);
        emit finished();
    }
    m_reply->deleteLater();
    m_reply = 0;
}
