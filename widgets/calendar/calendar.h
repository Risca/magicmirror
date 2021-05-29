#pragma once

#include <QHash>
#include <QList>
#include <QLocale>
#include <QSharedPointer>
#include <QTimer>
#include <QWidget>

class QDate;
class QNetworkAccessManager;

namespace Ui { class Calendar; }

namespace calendar {

class ISource;
struct Event;

QDate CurrentMonth(const QLocale& locale = QLocale());
QDate NextMonth(const QDate &thisMonth, const QLocale &locale = QLocale());

class Calendar : public QWidget
{
    Q_OBJECT

public:
    static bool Create(Calendar*& cal, QSharedPointer<QNetworkAccessManager> net, QWidget *parent = 0);
    virtual ~Calendar();

signals:
    void setEvents(const QList<Event>&);

public slots:
    void changeDay(const QDate& day);

private:
    Calendar(const QList<ISource*> &dataSources, QWidget *parent = NULL);
    void aggregateEvents(ISource* source, const QList<Event>& events);

    Ui::Calendar *ui;
    QList<ISource*> m_sources;
    QHash<ISource*, QList<Event> > m_events;
    QTimer m_timer;
};

} // namespace calendar
