#pragma once

#include "departure.h"

#include <QObject>

namespace departure {

class ISource  : public QObject
{
    Q_OBJECT

public:
    virtual ~ISource() = 0;

signals:
    void newDepartureTimes(QList<departure::Departure> times);

public slots:
    virtual void requestNewDepartures() = 0;

protected:
    ISource(QObject* parent) : QObject(parent) {}
    Q_DISABLE_COPY(ISource)
};
inline ISource::~ISource() {}

} // namespace departure
