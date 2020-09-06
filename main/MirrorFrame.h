/*
    This file is part of MagicMirror.
    MagicMirror is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    MagicMirror is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with MagicMirror.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MIRRORFRAME_H__
#define __MIRRORFRAME_H__

#include "weathericon.h"

namespace domoticz {
class Sensor;
}

#include <QFrame>
#include <QSharedPointer>
#include <QString>
#include <QTimer>
#include <QVector>

class CalendarInterface;
class WeatherData;
class QJsonObject;
class QLabel;
class QNetworkAccessManager;
class QStringList;

namespace Ui {
class MirrorFrame;
}

#define CALEVENTS_TIMEOUT	(1000 * 60 * 60 * 1)
#define FORECAST_TIMEOUT	(1000 * 60 * 60 * 4)
#define CURRENT_TIMEOUT		(1000 * 60 * 5)
#define TEMPERATURE_TIMEOUT (1000 * 60 * 5)

class MirrorFrame : public QFrame {
    Q_OBJECT
public:
    static MirrorFrame* Create();
    virtual ~MirrorFrame();

public slots:
    void calendarEventsDone(const QStringList &events);
    void calendarEventsError(const QString &error);
    void weatherDataError(const QString &error);
    void weatherEventsDone();
    void currentHumidity(double);
    void currentSkyConditions(const QString &sky);
    void currentTemperature(double);
    void currentWindSpeed(double);
    void sunrise(qint64);
    void sunset(qint64);
    void forecastEntry(const QJsonObject &);
    void forecastEntryCount(int);
    void updateClock();
    void indoorTemperature(const QString&, const QString&, const QDateTime &updated);
    void iconDownloaded(const QString &icon);
    void currentIcon(const QString &id);

private:
    MirrorFrame(QSharedPointer<QNetworkAccessManager> net);
    void createWeatherSystem();
    void createCalendarSystem();

    Ui::MirrorFrame *ui;
    QTimer m_calendarTimer;
    QTimer m_forecastTimer;
    QTimer m_currentWeatherTimer;
    QTimer m_clockTimer;
    QTimer m_localTempTimer;

    QSharedPointer<QNetworkAccessManager> m_net;
    QVector<QString> m_forecastIcons;
    QString m_currentIconId;
    WeatherIcon m_iconCache;

    WeatherData *m_weatherEvent;
    CalendarInterface *m_calendarEvent;
    domoticz::Sensor* m_indoorTempSensor;

    int m_forecastIndex;
    int m_forecastEntryCount;
};

#endif /* __MIRRORFRAME_H__ */
