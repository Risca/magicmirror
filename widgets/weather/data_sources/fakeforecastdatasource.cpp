#include "fakeforecastdatasource.h"

#include "utils/sensordata.h"

#include <QTimer>

namespace weather {

FakeForecastData::FakeForecastData(QObject *parent)
    : IForecastDataSource(parent)
{
    generateNewFakeData();
    QTimer::singleShot(1000, this, SLOT(generateNewFakeData()));
}

const QList<utils::SensorData> &FakeForecastData::forecast() const
{
    return m_currentForecast;
}

void FakeForecastData::generateNewFakeData()
{
    const int HOURS = 60 * 60;
    const QDateTime now = QDateTime::currentDateTime();
    QList<utils::SensorData> forecast;
    utils::SensorData d;

    d.timestamp = now;
    d.values[utils::TEMPERATURE] = 27;
    d.values[utils::HUMIDITY] = 15;
    forecast.push_back(d);

    d = utils::SensorData();
    d.timestamp = now.addSecs(6 * HOURS);
    d.values[utils::TEMPERATURE] = 21;
    d.values[utils::HUMIDITY] = 0;
    forecast.push_back(d);

    d = utils::SensorData();
    d.timestamp = now.addSecs(12 * HOURS);
    d.values[utils::TEMPERATURE] = 18;
    d.values[utils::HUMIDITY] = 6;
    d.values[utils::PRECIPITATION] = 2;
    forecast.push_back(d);

    d = utils::SensorData();
    d.timestamp = now.addSecs(24 * HOURS);
    d.values[utils::TEMPERATURE_HIGH] = 24;
    d.values[utils::TEMPERATURE_LOW] = 18;
    d.values[utils::HUMIDITY] = 70;
    d.values[utils::PRECIPITATION] = 8;
    forecast.push_back(d);

    d = utils::SensorData();
    d.timestamp = now.addSecs(48 * HOURS);
    d.values[utils::TEMPERATURE_HIGH] = 27;
    d.values[utils::TEMPERATURE_LOW] = 18;
    d.values[utils::HUMIDITY] = 10;
    d.values[utils::PRECIPITATION] = 0;
    forecast.push_back(d);

    m_currentForecast = forecast;
    emit newForecastAvailable();
}

} // namespace weather
