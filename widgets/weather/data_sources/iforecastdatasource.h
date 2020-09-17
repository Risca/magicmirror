#ifndef WEATHER_IFORECASTDATASOURCE_H
#define WEATHER_IFORECASTDATASOURCE_H

#include "weatherdata.h"

#include <QList>
#include <QObject>
#include <QSharedPointer>

class QNetworkAccessManager;
class QSettings;

namespace weather {

class IForecastDataSource : public QObject
{
    Q_OBJECT

public:
    static bool Create(IForecastDataSource*& obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject* parent = 0);
    virtual ~IForecastDataSource() {}

    virtual const QList<Data>& forecast() const = 0;

signals:
    void newForecastAvailable();

protected:
    IForecastDataSource(QObject* parent) : QObject(parent) {}
    Q_DISABLE_COPY(IForecastDataSource)
};

}

#endif // WEATHER_IFORECASTDATASOURCE_H
