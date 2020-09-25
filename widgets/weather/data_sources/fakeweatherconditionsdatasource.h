#ifndef FAKEWEATHERCONDITIONSDATASOURCE_H
#define FAKEWEATHERCONDITIONSDATASOURCE_H

#include "icurrentconditionsdatasource.h"
#include <QObject>

namespace weather {

class FakeWeatherConditionsDataSource : public ICurrentConditionsDataSource
{
    Q_OBJECT
public:
    ~FakeWeatherConditionsDataSource() {}

private:
    Q_DISABLE_COPY(FakeWeatherConditionsDataSource);
    FakeWeatherConditionsDataSource(QObject *parent);

    friend class ICurrentConditionsDataSource;

private slots:
    void generateNewFakeData();
};

} // namespace weather

#endif // FAKEWEATHERCONDITIONSDATASOURCE_H
