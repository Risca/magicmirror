#include "fakeweatherconditionsdatasource.h"

#include <QDebug>
#include <QTimer>

weather::FakeWeatherConditionsDataSource::FakeWeatherConditionsDataSource(QObject *parent) :
    ICurrentConditionsDataSource(parent)
{
    QTimer::singleShot(1000, this, SLOT(generateNewFakeData()));
}

void weather::FakeWeatherConditionsDataSource::generateNewFakeData()
{
    emit temperature(9000.1);

    QPixmap noWeather(":/weather/icons/no-weather-icon.png");
    emit image(noWeather);
}
