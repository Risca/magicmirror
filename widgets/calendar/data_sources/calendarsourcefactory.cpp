#include "isource.hpp"
#include "icssource.h"
#include "nosource.h"

#include <QSettings>

namespace calendar {

bool ISource::Create(ISource *&obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    if (settings->value("type").toString() == "ics") {
        return IcsSource::Create(obj, settings, net, parent);
    }
    return NoSource::Create(obj, parent);
}

} // namespace calendar
