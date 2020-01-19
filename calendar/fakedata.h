#ifndef FAKEDATA_H
#define FAKEDATA_H

#include "calendarinterface.h"

class FakeData : public CalendarInterface
{
    Q_OBJECT

public:
    explicit FakeData(QObject* parent = 0);
    virtual ~FakeData();

public slots:
    void sync();

protected:
    Q_DISABLE_COPY(FakeData)
};

#endif // FAKEDATA_H
