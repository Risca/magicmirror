#pragma once

/*
 * This file is named cal_nosource.h to not confuse Qt moc with sensor::NoSource
 */

#include "cal_isource.h"

#include <QObject>

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
    NoSource(QObject* parent) : ISource(parent) { /* empty */ }
    Q_DISABLE_COPY(NoSource)
};

} // namespace calendar
