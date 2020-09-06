#ifndef EVENT_HPP
#define EVENT_HPP

#include <QDate>
#include <QString>

namespace calendar {

struct Event {
    QDate start;
    QDate stop;
    bool allDayEvent;
    QString summary;

    bool operator<(const Event& other) const {
        if (this->start == other.start) {
            return this->stop < other.stop;
        }
        else {
            return this->start < other.start;
        }
    }
    Event() : allDayEvent(false) {}
};

} // namespace calendar

#endif // EVENT_HPP
