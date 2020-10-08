#pragma once

#include "iforecastdatasource.h"

namespace weather {

class FakeForecastData : public IForecastDataSource
{
    Q_OBJECT

public:
    virtual ~FakeForecastData() {}

    const QList<utils::SensorData>& forecast() const;

private:
    Q_DISABLE_COPY(FakeForecastData);
    explicit FakeForecastData(QObject *parent = NULL);

    friend class IForecastDataSource;

    QList<utils::SensorData> m_currentForecast;

private slots:
    void generateNewFakeData();
};

} // namespace weather
