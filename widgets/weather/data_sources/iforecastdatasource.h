#pragma once

#include "utils/sensordata.h"

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
    virtual ~IForecastDataSource() = 0;

    virtual const QList<utils::SensorData>& forecast() const = 0;

signals:
    void newForecastAvailable();

protected:
    IForecastDataSource(QObject* parent) : QObject(parent) {}
    Q_DISABLE_COPY(IForecastDataSource)
};
inline IForecastDataSource::~IForecastDataSource() {}

}
