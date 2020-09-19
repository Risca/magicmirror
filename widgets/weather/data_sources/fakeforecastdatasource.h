#ifndef FAKEFORECASTDATASOURCE_H
#define FAKEFORECASTDATASOURCE_H

#include "iforecastdatasource.h"

namespace weather {

class FakeForecastData : public IForecastDataSource
{
    Q_OBJECT

public:
    virtual ~FakeForecastData() {}

    const QList<Data>& forecast() const;

private:
    Q_DISABLE_COPY(FakeForecastData);
    explicit FakeForecastData(QObject *parent = NULL);

    friend class IForecastDataSource;

    QList<Data> m_currentForecast;

private slots:
    void generateNewFakeData();
};

} // namespace weather

#endif // FAKEFORECASTDATASOURCE_H
