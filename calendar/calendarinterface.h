#ifndef CALENDAR_INTERFACE_H
#define CALENDAR_INTERFACE_H

#include <QObject>
#include <QSharedPointer>

class QNetworkAccessManager;
class QString;
class QStringList;

class CalendarInterface : public QObject {
    Q_OBJECT

public:
    static bool Create(CalendarInterface*& cal, QSharedPointer<QNetworkAccessManager> net, QObject* parent = 0);
    virtual ~CalendarInterface() {}

public slots:
    virtual void sync() = 0;

signals:
    void finished(const QStringList&);
    void error(const QString&);

protected:
    CalendarInterface(QObject* parent) : QObject(parent) {}
    Q_DISABLE_COPY(CalendarInterface)
};

#endif // CALENDAR_INTERFACE_H
