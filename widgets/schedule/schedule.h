#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QList>
#include <QSharedPointer>
#include <QWidget>

class QNetworkAccessManager;
class QString;

namespace Ui {
class Schedule;
}

namespace calendar {
class ISource;
class Event;
}

namespace schedule {

class Schedule : public QWidget
{
    Q_OBJECT

public:
    static bool Create(Schedule*& obj, QSharedPointer<QNetworkAccessManager>& net, QWidget* parent = 0);
    ~Schedule();

private:
    explicit Schedule(calendar::ISource *source, const QString& title, QWidget *parent = 0);

    Ui::Schedule *ui;
    calendar::ISource *m_source;

private slots:
    void syncFinished(const QList<calendar::Event>& events);
};

} // namespace schedule

#endif // SCHEDULE_H
