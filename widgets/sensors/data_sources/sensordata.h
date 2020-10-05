#pragma once

#include <QDateTime>
#include <QString>

namespace sensors {

struct SensorData
{
    QString name;
    QString value;
    QDateTime lastUpdated;

    bool operator<(const SensorData& other) const { return this->name < other.name; }
};

} // namespace sensors
