#include "cal_isource.h"
#include "icssource.h"
#include "cal_nosource.h"

#include <QDebug>
#include <QSettings>

namespace calendar {

bool ISource::Create(ISource *&obj, QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    bool objectCreated;

    settings->beginGroup("ICS");
    objectCreated = IcsSource::Create(obj, settings, net, parent);
    settings->endGroup();
    if (objectCreated) {
        return true;
    }

    qWarning() << "No calendar data source found";

    return NoSource::Create(obj, parent);
}

} // namespace calendar
