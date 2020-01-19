#ifndef CALENDAR_INTERFACE_H
#define CALENDAR_INTERFACE_H

#include <QObject>

class QString;

class CalendarInterface : public QObject {
    Q_OBJECT

public:
    static bool Create(CalendarInterface*& cal, QObject* parent = 0);
    virtual ~CalendarInterface() {}

public slots:
    virtual void sync() = 0;

signals:
    void newEvent(const QString&);
    void finished();
    void error(const QString&);

protected:
    CalendarInterface(QObject* parent) : QObject(parent) {}
    Q_DISABLE_COPY(CalendarInterface)
};

#endif // CALENDAR_INTERFACE_H