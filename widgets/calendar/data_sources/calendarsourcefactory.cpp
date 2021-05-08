#include "cal_isource.h"
#include "icssource.h"
#include "googlecalendarsource.h"
#include "cal_nosource.h"

#include <QDebug>
#include <QList>
#include <QSettings>

namespace calendar {

bool ISource::Create(QList<ISource*> &objs, QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    bool objectCreated;
    ISource* obj;

    settings->beginGroup("ICS");
    objectCreated = IcsSource::Create(obj, settings, net, parent);
    settings->endGroup();
    if (objectCreated) {
        objs.push_back(obj);
    }

    settings->beginGroup("Google");
    objectCreated = GoogleCalendarSource::Create(obj, settings, net, parent);
    settings->endGroup();
    if (objectCreated) {
        objs.push_back(obj);
    }

    if (objs.empty()) {
        qWarning() << "No calendar data source found";

        if (NoSource::Create(obj, parent)) {
            objs.push_back(obj);
        }
    }

    return !objs.empty();
}

} // namespace calendar
