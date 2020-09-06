#include "icssource.h"
#include "event.hpp"

#include <libical/ical.h>

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPair>
#include <QSettings>
#include <QString>

#include <algorithm>
#include <time.h>

namespace calendar {

#define DEFAULT_RETRY_TIMEOUT (1000 * 30)

typedef QPair<QDate, QDate> StartStopDate;

namespace {

QDate IcalDatePropertyToQDate(icalproperty* prop, bool& isDate)
{
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
            return QDate();
        }
    }

    return QDateTime::fromTime_t(icaltime_as_timet(t)).date();
}

StartStopDate GetEventStartStopDates(icalcomponent* c)
{
    StartStopDate dates;
    icalproperty* prop = icalcomponent_get_first_property(c, ICAL_DTSTART_PROPERTY);
    if (prop != 0) {
        bool isDate;
        dates.first = IcalDatePropertyToQDate(prop, isDate);
        prop = icalcomponent_get_first_property(c, ICAL_DTEND_PROPERTY);
        if (prop != 0) {
            dates.second = IcalDatePropertyToQDate(prop, isDate);
            if (isDate) {
                // trial and error show that if DTEND is a normal date (without time),
                // then it's commonly set to 00:00 (UTC) the day after the last day.
                dates.second = dates.second.addDays(-1);
            }
        }
    }
    return dates;
}

QString GetSummary(icalcomponent* c)
{
    return icalcomponent_get_summary(c);
}

bool IsFutureEvent(const StartStopDate& dates, const QDate& today)
{
    return dates.first >= today || (dates.second.isValid() && dates.second >= today);
}

} // anonymous namespace

bool IcsSource::Create(calendar::ISource *&obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    if (settings->value("type").toString() == "ics") {
        const QUrl url = settings->value("url").toUrl();
        if (url.isValid()) {
            obj = new IcsSource(url, net, parent);
            return !!obj;
        }
    }
    return false;
}

void IcsSource::sync()
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

IcsSource::IcsSource(const QUrl &url, QSharedPointer<QNetworkAccessManager> net, QObject *parent) :
    ISource(parent),
    m_net(net),
    m_reply(0),
    m_url(url)
{
    qDebug() << "Using ICS source:" << m_url.toString();
    m_retryTimer.setInterval(DEFAULT_RETRY_TIMEOUT);
    m_retryTimer.setSingleShot(true);
    connect(&m_retryTimer, SIGNAL(timeout()), this, SLOT(sync()));
    if (!m_url.isLocalFile()) {
        m_retryTimer.start();
    }
}

void IcsSource::downloadFinished()
{
    if (m_reply->error()) {
        qWarning() << __PRETTY_FUNCTION__ << ":" << m_reply->errorString();
        m_retryTimer.start();
    }
    else {
        QList<calendar::Event> events;
        const QDate today = QDate::currentDate();
        icalcomponent* comp = icalparser_parse_string(m_reply->readAll().data());
        if (comp != 0) {
            for (icalcomponent* c = icalcomponent_get_first_component(comp, ICAL_VEVENT_COMPONENT);
                 c != 0;
                 c = icalcomponent_get_next_component(comp, ICAL_VEVENT_COMPONENT))
            {
                const StartStopDate dates = GetEventStartStopDates(c);
                if (IsFutureEvent(dates, today)) {
                    calendar::Event event;
                    event.start = dates.first;
                    if (dates.second.isValid() && dates.first != dates.second) {
                        event.allDayEvent = false;
                        event.stop = dates.second;
                    }
                    else {
                        event.allDayEvent = true;
                    }
                    event.summary = GetSummary(c);
                    events << event;
                }
            }
        }
        icalcomponent_free(comp);
        std::sort(events.begin(), events.end());
        emit finished(events);
    }
    m_reply->deleteLater();
    m_reply = 0;
}

} // namespace calendar
