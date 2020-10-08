#pragma once

/*
 * This file is named sensor_isource.h to not confuse Qt moc with calendar::ISource
 */

#include <QList>
#include <QObject>
#include <QSharedPointer>

class QNetworkAccessManager;
class QSettings;
class QString;

namespace utils {

struct SensorData;

}

namespace sensors {

class ISource : public QObject
{
    Q_OBJECT

public:
    static bool Create(ISource*& obj, QSharedPointer<QSettings> &settings, QSharedPointer<QNetworkAccessManager> net, QObject* parent = 0);
    virtual ~ISource() = 0;

signals:
    void sensorDataUpdated(const QList<utils::SensorData>& data);
    void error(const QString&);

protected:
    ISource(QObject *parent) : QObject(parent) {}
    Q_DISABLE_COPY(ISource)
};
inline ISource::~ISource() {}

} // namespace sensors
