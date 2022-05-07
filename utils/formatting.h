#pragma once

#include <QDate>
#include <QLocale>
#include <QString>

namespace utils {

inline QString Temperature(double t)
{
    return QString::number(t, 'f', 1) + QString::fromUtf8("°");
}

inline QString Humidity(double h)
{
    return QString::number(h, 'f', 0) + "%";
}

inline QString Precipitation(double p)
{
    return QString::number(p, 'f', 0) + " mm";
}

inline QString Capitalized(const QString& str, const QLocale& locale = QLocale())
{
    QString firstLetter = str.at(0);
    return locale.toUpper(firstLetter) + str.right(str.size() - 1);
}

inline QString Date(const QDateTime& dt, int laterHours = 16)
{
    const qint64 HOURS = 3600;
    const QDateTime later = QDateTime::currentDateTime().addSecs(laterHours * HOURS);
    const QLocale locale;

    if (dt >= later) {
        QString day = locale.toString(dt.date(), "ddd");
        return Capitalized(day);
    }
    else {
        return locale.toString(dt.time(), QLocale::ShortFormat);
    }
    return QString();
}

}
