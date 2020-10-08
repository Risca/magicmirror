#pragma once

#include <QDateTime>
#include <QIcon>
#include <QMap>

namespace utils {

enum SensorDataKey {
    TEMPERATURE,
    TEMPERATURE_LOW,
    TEMPERATURE_HIGH,
    HUMIDITY,
    WIND_SPEED,
    PRECIPITATION,
};

struct SensorData
{
    QDateTime timestamp;
    QIcon icon;
    QMap<SensorDataKey, double> values;

    bool operator<(const SensorData& other) const { return this->timestamp < other.timestamp; }
};

} // namespace utils
