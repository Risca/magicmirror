#pragma once

#include <QDateTime>
#include <QString>

namespace sensors {

struct SensorData
{
    QString name;
    QString value;
    QDateTime lastUpdated;
};

} // namespace sensors
