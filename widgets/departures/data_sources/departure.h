#pragma once

#include <QString>
#include <QDateTime>

namespace departure {

struct Departure {
    quint64 stopID;
    QString stopName;
    QString transport;
    QDateTime time;

    bool operator<(const Departure& other) const {
        return this->time < other.time;
    }
};

} // namespace departure
