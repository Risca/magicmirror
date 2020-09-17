#include "icurrentconditionsdatasource.h"

#include "iforecastdatasource.h"
#include "fakeforecastdatasource.h"
#include "openweathermapforecastdatasource.h"
#include "openweathermapconditionsdatasource.h"

#include <QDebug>
#include <QSettings>

namespace weather {

bool IForecastDataSource::Create(IForecastDataSource *&obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject *parent)
{
    const QString appId = settings->value("appid").toString();
    const QString townId = settings->value("townid").toString();
    if (!appId.isEmpty() && !townId.isEmpty()) {
        obj = new OpenWeatherMapForecastDataSource(appId, townId, net, parent);
    }
    else {
        qWarning() << "Falling back to fake forecast data";
        obj = new FakeForecastData(parent);
    }
    return !!obj;
}

bool ICurrentConditionsDataSource::Create(ICurrentConditionsDataSource*& obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject* parent)
{
    const QString appId = settings->value("appid").toString();
    const QString townId = settings->value("townid").toString();
    if (!appId.isEmpty() && !townId.isEmpty()) {
        obj = new OpenWeatherMapConditionsDataSource(appId, townId, net, parent);
    }
    return !!obj;
}

} // namespace weather
