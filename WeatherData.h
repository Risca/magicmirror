#ifndef __WEATHERDATA_H__
#define __WEATHERDATA_H__

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>

class QNetworkAccessManager;
class QNetworkReply;
class QThread;

class WeatherData : public QObject
{
    Q_OBJECT
public:
    static bool Create(WeatherData*& weatherData, QSharedPointer<QNetworkAccessManager> net, QObject *parent = 0);
    virtual ~WeatherData();

    void setThreadPointer(QThread *t) { m_thread = t; }

signals:
    void currentConditions(QMap<QString,QString>);
    void forecastEntry(QJsonObject);
    void finished();
    void error(QString);
    void temperature(double);
    void humidity(double);
    void windSpeed(double);
    void skyConditions(QString);
    void sunrise(qint64);
    void sunset(qint64);
    void forecastEntryCount(int);
    void currentIcon(QString);

public slots:
    void processForecast();
    void processCurrentWeather();
    void forecastReplyFinished();
    void currentReplyFinished();

private:
    Q_DISABLE_COPY(WeatherData);
    WeatherData(const QString& appId, const QString& townId, QSharedPointer<QNetworkAccessManager> net, QObject *parent);

    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply *m_forecast;
    QNetworkReply *m_current;
    const QString m_appID;
    const QString m_townID;
    QThread *m_thread;
};

#endif
