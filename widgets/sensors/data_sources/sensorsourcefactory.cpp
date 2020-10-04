#include "sensor_isource.h"

#include "sensor_nosource.h"
#include "domoticzsensor.h"

#include <QDebug>
#include <QSettings>
#include <QSharedPointer>

namespace sensors {

bool ISource::Create(ISource*& obj, QSharedPointer<QSettings>& settings, QSharedPointer<QNetworkAccessManager> net, QObject* parent)
{
    bool ok;

    settings->beginGroup("Domoticz");
    ok = DomoticzSensor::Create(obj, settings, net, parent);
    settings->endGroup();

    if (ok) {
        return true;
    }

    qWarning() << "No sensor sources";

    return NoSource::Create(obj, parent);
}

} // namespace sensors
