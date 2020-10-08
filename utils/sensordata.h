#pragma once

#include <QDateTime>
#include <QIcon>
#include <QMap>
#include <QString>

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
    QString source;
    QDateTime timestamp;
    QIcon icon;
    QMap<SensorDataKey, double> values;

    // Helpers (use the typedef's below!)
    template <typename T, T SensorData::* M>
    struct SensorDataComparator
    {
        bool operator()(const SensorData& left, const SensorData& right)
        {
            return left.*M < right.*M;
        }
    };

    typedef SensorDataComparator<QString, &SensorData::source> SortbySource;
    typedef SensorDataComparator<QDateTime, &SensorData::timestamp> SortByAge;
};

} // namespace utils
