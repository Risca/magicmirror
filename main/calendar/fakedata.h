#ifndef FAKEDATA_H
#define FAKEDATA_H

#include "calendarinterface.h"

class FakeData : public CalendarInterface
{
    Q_OBJECT

public:
    static bool Create(CalendarInterface*& cal, QObject* parent);
    virtual ~FakeData();

public slots:
    void sync();

protected:
    FakeData(QObject* parent);
    Q_DISABLE_COPY(FakeData)
};

#endif // FAKEDATA_H
