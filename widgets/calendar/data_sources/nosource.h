#ifndef CALENDAR_NOSOURCE_H
#define CALENDAR_NOSOURCE_H

#include "isource.hpp"

#include <QObject>
#include <QSharedPointer>

class QNetworkAccessManager;
class QSettings;

namespace calendar {

class NoSource : public ISource
{
    Q_OBJECT

public:
    static bool Create(ISource*& obj, QObject* parent = 0)
    {
        obj = new NoSource(parent);
        return !!obj;
    }

    virtual ~NoSource() {}

public slots:
    virtual void sync() {}

protected:
    Q_DISABLE_COPY(NoSource)

    NoSource(QObject* parent) : ISource(parent) { /* empty */ }
};

} // namespace calendar

#endif // CALENDAR_NOSOURCE_H
