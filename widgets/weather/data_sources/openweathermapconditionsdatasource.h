#ifndef OPENWEATHERMAPCONDITIONSDATASOURCE_H
#define OPENWEATHERMAPCONDITIONSDATASOURCE_H

#include "icurrentconditionsdatasource.h"

#include <QObject>
#include <QSharedPointer>
#include <QTimer>

class IconCache;
class QNetworkAccessManager;
class QNetworkReply;

namespace weather {

class OpenWeatherMapConditionsDataSource : public ICurrentConditionsDataSource
{
    Q_OBJECT
public:
    ~OpenWeatherMapConditionsDataSource();

protected slots:
    void requestWeatherConditions();
    void requestFinished();
    void iconDownloaded(const QString& icon);

protected:
    Q_DISABLE_COPY(OpenWeatherMapConditionsDataSource);
    OpenWeatherMapConditionsDataSource(const QString& appId, const QString& townId, QSharedPointer<QNetworkAccessManager> net, QObject *parent);

    friend class ICurrentConditionsDataSource;

    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply *m_reply;
    const QString m_appID;
    const QString m_townID;

    QTimer m_timer;

    IconCache* m_iconCache;
};

} // namespace weather

#endif // OPENWEATHERMAPCONDITIONSDATASOURCE_H
