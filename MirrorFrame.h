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

#include <QFrame>
#include <QSharedPointer>
#include <QString>
#include <QTimer>
#include <QVector>

#ifdef __USE_RPI__
#include <th02.h>
#endif
#include "calendar/calendarinterface.h"
#include "WeatherData.h"
#include "weathericon.h"

class QJsonObject;
class QLabel;
class QNetworkAccessManager;

namespace Ui {
class MirrorFrame;
}

#define CALEVENTS_TIMEOUT	(1000 * 60 * 60 * 1)
#define FORECAST_TIMEOUT	(1000 * 60 * 60 * 4)
#define CURRENT_TIMEOUT		(1000 * 60 * 5)
#define TWELVE_HOURS        (1000 * 60 * 60 * 12)
#define THIRTY_MINUTES      (1000 * 60 * 30)

class MirrorFrame : public QFrame {
    Q_OBJECT
public:
    static MirrorFrame* Create();

    virtual ~MirrorFrame();
    void registerTouchEvent();

public slots:
    void calendarEventsDone();
    void calendarEventsError(const QString &error);
    void calendarEventsEvent(const QString &s);
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
    void updateLocalTemp();
    void iconDownloaded(const QString &icon);
    void currentIcon(const QString &id);
    void messageReceivedOnTopic(const QString &t, const QString &p);
    void connectionComplete();
    void disconnectedEvent();
    void lightningTimeout();


signals:
    void touchDetected();

private:
    MirrorFrame(QSharedPointer<QNetworkAccessManager> net);
    void deleteCalendarEventsList();
    void createWeatherSystem();
    void createCalendarSystem();
    void setupMqttSubscriber();

    Ui::MirrorFrame *ui;
    QTimer m_calendarTimer;
    QTimer m_forecastTimer;
    QTimer m_currentWeatherTimer;
    QTimer m_clockTimer;
    QTimer m_localTempTimer;
    QTimer m_lightningTimer;

    QSharedPointer<QNetworkAccessManager> m_net;
    QVector<QString> m_forecastIcons;
    QString m_currentIconId;
    WeatherIcon m_iconCache;

    WeatherData *m_weatherEvent;
    CalendarInterface *m_calendarEvent;

    int m_forecastIndex;
    int m_forecastEntryCount;
    bool m_newEventList;
};

#endif /* __MIRRORFRAME_H__ */
