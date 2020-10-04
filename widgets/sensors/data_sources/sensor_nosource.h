#pragma once

/*
 * This file is named sensor_nosource.h to not confuse Qt moc with calendar::NoSource
 */

#include "sensor_isource.h"

#include <QObject>

namespace sensors {

class NoSource : public ISource
{
    Q_OBJECT

public:
    static bool Create(ISource*& obj, QObject* parent)
    {
        obj = new NoSource(parent);
        return !!obj;
    }

    virtual ~NoSource() {}

protected:
    NoSource(QObject* parent) : ISource(parent) {}
    Q_DISABLE_COPY(NoSource)
};

} // namespace sensors
