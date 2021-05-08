#pragma once

#include <QColor>
#include <QDate>
#include <QString>

namespace calendar {

struct Event {
    QDate start;
    QDate stop;
    QString summary;
    QColor color;

    bool operator<(const Event& other) const {
        if (this->start == other.start) {
            return this->stop < other.stop;
        }
        else {
            return this->start < other.start;
        }
    }

    bool operator==(const QDate& date) const {
        return this->start <= date && date <= this->stop;
    }
};

} // namespace calendar
