#include "CalendarData.h"

CalendarData::CalendarData(QObject *parent) : QObject(parent)
{
}

CalendarData::~CalendarData()
{
}

void CalendarData::process()
{
    QProcess process;
    QByteArray results;

    qDebug() << __PRETTY_FUNCTION__;
    process.start("python3", QStringList() << "/usr/bin/magicmirror.py");
    if (!process.waitForFinished(120000)) {
        qWarning() << __PRETTY_FUNCTION__ << ": Error running magicmirror.py";
        emit error(QString("Error running magicmirror.py"));
    }
    results = process.readAllStandardOutput();

    processResults(results);
    emit finished();
}

/*
2017-08-26T13:30:00-05:00 Jonathan's 1st Birthday Party
2017-08-29T15:30:00-05:00 Eye Doctor Appt
2017-08-29T18:00:00-05:00 Fantasy Football Draft
2017-08-31 Vacation - Mary/Nate & Debbie/Cory Wedding!!
2017-09-02T08:00:00-05:00 Maddie - Hot Shot Tots
2017-09-03T12:00:00-05:00 It Takes Two times Two!
2017-09-09T08:00:00-05:00 Maddie - Hot Shot Tots
2017-09-12T18:30:00-05:00 Norah - Parent & Tot Fall Adventure
2017-09-16T08:00:00-05:00 Maddie - Hot Shot Tots
2017-09-23T08:00:00-05:00 Maddie - Hot Shot Tots
*/
void CalendarData::processResults(QByteArray &results)
{
    QDateTime start;
    QList<QByteArray> events = results.split('\n');
    qDebug() << __PRETTY_FUNCTION__;
    if (results.contains("Go to the following link in your browser")) {
        emit newEvent(QString("Please run from command line to authenticate"));
    }
    else {
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
    }
}

