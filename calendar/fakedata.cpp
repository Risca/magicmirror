#include "fakedata.h"

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QString>
#include <QTimeZone>

#if 0
bool CalendarInterface::Create(CalendarInterface*& cal, QSharedPointer<QNetworkAccessManager> net, QObject* parent)
{
    Q_UNUSED(net);
    cal = new FakeData(parent);
    return !!cal;
}
#endif

FakeData::FakeData(QObject *parent) :
    CalendarInterface(parent)
{
    // empty
}

FakeData::~FakeData()
{
    // empty
}

void FakeData::sync()
{
    const QByteArray results =
                "2020-01-31 Familjen Frost i Norrköping\n" \
                "2020-02-03T19:30:00+01:00 Babymetal - VIP\n" \
                "2020-08-01 RAMMSTEIN - Europe Stadium Tour 2020 - dag 2\n";

    qDebug() << __PRETTY_FUNCTION__;

    QDateTime start;
    QList<QByteArray> events = results.split('\n');

    for (int i = 0; i < events.size(); i++) {
        if (events.at(i).size() == 0)
            continue;

        if (events.at(i).contains("Getting the upcoming"))
            continue;

        QString eventTime = events.at(i).left(events.at(i).indexOf(' '));
        QString eventDescription = events.at(i).right(events.at(i).size() - events.at(i).indexOf(' '));
        if (eventTime.indexOf("T") != -1) {
            start = QDateTime::fromString(eventTime.left(19), "yyyy-MM-dd'T'HH:mm:ss");
            QString tzb = eventTime.mid(19, 3);
            QTimeZone tz(tzb.toInt() * 60 * 1000 * 60);
            if (tz.isValid())
                start.setTimeZone(tz);
            QString event(start.toString("h:mm ap 'on' dddd, MMMM dd") + QString(" : ") + eventDescription.trimmed());
            emit newEvent(event);
        }
        else {
            start = QDateTime::fromString(eventTime, "yyyy-MM-dd");
            QString event(start.toString("dddd, MMMM dd") + QString(" : ") + eventDescription.trimmed());
            emit newEvent(event);
        }
    }

    emit finished();
}
