#include "calendarinterface.h"

#include "fakedata.h"
#include "icscalendar.h"
#include "settingsfactory.h"

bool CalendarInterface::Create(CalendarInterface*& cal, QSharedPointer<QNetworkAccessManager> net, QObject* parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Calendar");
    if (settings->value("type").toString() == "ics" && IcsCalendar::Create(cal, net, parent)) {
        return true;
    }
    return FakeData::Create(cal, parent);
}
