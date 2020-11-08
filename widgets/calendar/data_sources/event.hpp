#ifndef CALENDAR_EVENT_HPP
#define CALENDAR_EVENT_HPP

#include <QDate>
#include <QString>

namespace calendar {

struct Event {
    QDate start;
    QDate stop;
    QString summary;

    bool operator<(const Event& other) const {
        if (this->start == other.start) {
            return this->stop < other.stop;
        }
        else {
            return this->start < other.start;
        }
    }
};

} // namespace calendar

#endif // CALENDAR_EVENT_HPP
