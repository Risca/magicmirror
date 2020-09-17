#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QDate>
#include <QIcon>
#include <QMap>

namespace weather {

enum WeatherDataKey {
    TEMPERATURE,
    TEMPERATURE_LOW,
    TEMPERATURE_HIGH,
    HUMIDITY,
    WIND_SPEED,
    PRECIPITATION,
};

struct Data
{
    QDateTime dateTime;
    QIcon icon;
    QMap<WeatherDataKey, double> values;

    bool operator<(const Data& other) const { return this->dateTime < other.dateTime; }
};

} // namespace weather

#endif // WEATHERDATA_H
