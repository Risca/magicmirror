#pragma once

#include "data_sources/event.hpp"

#include <QCalendarWidget>
#include <QDate>
#include <QList>
#include <QMap>
#include <QSize>

namespace calendar {

class CustomCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    CustomCalendarWidget(QWidget *parent = 0);

    virtual QSize minimumSizeHint() const;

public slots:
    void setEvents(const QList<calendar::Event> &events);

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;

    QMap<QDate, QList<Event> > m_events;
};

} // namespace calendar
