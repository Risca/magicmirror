#pragma once

#include "iforecastdatasource.h"

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QTimer>

class IconCache;
class QNetworkAccessManager;
class QNetworkReply;

namespace weather {

class OpenWeatherMapForecastDataSource : public IForecastDataSource
{
    Q_OBJECT

public:
    virtual ~OpenWeatherMapForecastDataSource();

    const QList<utils::SensorData>& forecast() const;

public slots:
    void requestForecast();

protected slots:
    void forecastRequestFinished();
    void updateIcon(const QString& icon);

protected:
    Q_DISABLE_COPY(OpenWeatherMapForecastDataSource);
    OpenWeatherMapForecastDataSource(const QString& appId, const QString& townId, QSharedPointer<QNetworkAccessManager> net, QObject *parent);

    friend class IForecastDataSource;

    QList<utils::SensorData> m_currentForecast;
    QList<QString> m_forecastIcons;

    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply *m_reply;
    const QString m_appID;
    const QString m_townID;

    QTimer m_timer;

    IconCache* m_iconCache;
};

} // namespace weather
