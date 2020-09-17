#include "fakeforecastdatasource.h"

#include <QTimer>

namespace weather {

FakeForecastData::FakeForecastData(QObject *parent)
    : IForecastDataSource(parent)
{
    generateNewFakeData();
    QTimer::singleShot(1000, this, SLOT(generateNewFakeData()));
}

const QList<Data> &FakeForecastData::forecast() const
{
    return m_currentForecast;
}

void FakeForecastData::generateNewFakeData()
{
    const int HOURS = 60 * 60;
    const QDateTime now = QDateTime::currentDateTime();
    QList<Data> forecast;
    Data d;

    d.dateTime = now;
    d.values[TEMPERATURE] = 27;
    d.values[HUMIDITY] = 15;
    forecast.push_back(d);

    d = Data();
    d.dateTime = now.addSecs(6 * HOURS);
    d.values[TEMPERATURE] = 21;
    d.values[HUMIDITY] = 0;
    forecast.push_back(d);

    d = Data();
    d.dateTime = now.addSecs(12 * HOURS);
    d.values[TEMPERATURE] = 18;
    d.values[HUMIDITY] = 6;
    d.values[PRECIPITATION] = 2;
    forecast.push_back(d);

    d = Data();
    d.dateTime = now.addSecs(24 * HOURS);
    d.values[TEMPERATURE_HIGH] = 24;
    d.values[TEMPERATURE_LOW] = 18;
    d.values[HUMIDITY] = 70;
    d.values[PRECIPITATION] = 8;
    forecast.push_back(d);

    d = Data();
    d.dateTime = now.addSecs(48 * HOURS);
    d.values[TEMPERATURE_HIGH] = 27;
    d.values[TEMPERATURE_LOW] = 18;
    d.values[HUMIDITY] = 10;
    d.values[PRECIPITATION] = 0;
    forecast.push_back(d);

    m_currentForecast = forecast;
    emit newForecastAvailable();
}

} // namespace weather
