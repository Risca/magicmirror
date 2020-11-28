#include "customcalendarwidget.h"

#include <QDate>
#include <QDebug>
#include <QPainter>
#include <QRect>
#include <QString>

#include <algorithm>

namespace calendar {

CustomCalendarWidget::CustomCalendarWidget(QWidget *parent) :
    QCalendarWidget(parent)
{

}

QSize CustomCalendarWidget::minimumSizeHint() const
{
    // Workaround for bug in stylesheet. Without this, the right and bottom
    // edges gets clipped.
    return QCalendarWidget::minimumSizeHint() + QSize(2, 2);
}

void CustomCalendarWidget::setEvents(const QList<Event> &events)
{
    m_events.clear();
    foreach (const Event &e, events) {
        for (QDate date = e.start; date <= e.stop; date = date.addDays(1)) {
            m_events[date].push_back(e);
        }
    }
    this->update();
}

void CustomCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    QCalendarWidget::paintCell(painter, rect, date);

    const QList<calendar::Event> events = m_events.value(date);
    if (!events.empty()) {
        QPoint p = rect.topLeft();
        const int radii = 2;
        // x1 = w/2, if s == 1
        // x1 = w/2 - ((s - 1) * r + 1), if s > 1
        int x1;
        if (events.size() == 1) {
            x1 = rect.width() / 2;
        }
        else {
            x1 = rect.width() / 2 - ((events.size() - 1) * radii + 1);
        }
        p += QPoint(x1, rect.height() - radii);

        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);

        foreach (const calendar::Event &e, events) {
            painter->setBrush(QBrush(e.color));
            painter->drawEllipse(p, radii, radii);

            p += QPoint(2 * radii + 1, 0);
        }

        painter->restore();
    }
}

} // namespace calendar
