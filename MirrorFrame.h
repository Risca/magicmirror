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

#include <QtNetwork/QtNetwork>
#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#ifdef __USE_RPI__
    #include <th02.h>
#endif
#include "CalendarData.h"
#include "WeatherData.h"
#include "weathericon.h"

#define MONITOR_TIMEOUT		(1000 * 60 * 1)
#define CALEVENTS_TIMEOUT	(1000 * 60 * 60 * 1)
#define FORECAST_TIMEOUT	(1000 * 60 * 60 * 4)
#define CURRENT_TIMEOUT		(1000 * 60 * 5)
#define TWELVE_HOURS        (1000 * 60 * 60 * 12)
#define THIRTY_MINUTES      (1000 * 60 * 30)

class MirrorFrame : public QFrame {
	Q_OBJECT
public:
	MirrorFrame(QFrame *parent = 0);
	virtual ~MirrorFrame();
	void registerTouchEvent();

public slots:
	void getEvents();
	void getForecast();
	void getCurrentWeather();
	void calendarEventsDone();
	void calendarEventsError(QString);
	void calendarEventsEvent(QString);
	void weatherDataError(QString);
	void weatherEventsDone();
	void currentHumidity(double);
	void currentSkyConditions(QString);
	void currentTemperature(double);
	void currentWindSpeed(double);
	void sunrise(qint64);
	void sunset(qint64);
	void forecastEntry(QJsonObject);
    void forecastEntryCount(int);
	void updateClock();
	void monitorOn();
	void monitorOff();
	void resetMonitorTimer();
	void updateLocalTemp();
    void iconReplyFinished(QNetworkReply*);
    void currentIcon(QString);
    void messageReceivedOnTopic(QString, QString);
    void connectionComplete();
    void disconnectedEvent();
    void lightningTimeout();

    
signals:
	void touchDetected();

//protected:
//	void showEvent(QShowEvent*);

private:
    void deleteCalendarEventsList();
	void createStateMachine();
	void enableTimers();
	void turnMonitorOn();
	void turnMonitorOff();
    void createWeatherSystem();
    void createCalendarSystem();
    void getIcon(QString);
    void setupMqttSubscriber();

	QStateMachine *m_monitorState;
	QTimer *m_calendarTimer;
	QTimer *m_forecastTimer;
	QTimer *m_currentWeatherTimer;
	QTimer *m_clockTimer;
	QTimer *m_monitorTimer;
	QTimer *m_localTempTimer;
    QTimer *m_lightningTimer;
	QList<QLabel*> m_calendarEvents;
	QLabel *m_calLabel;
	QLabel *m_currentLabel;
	QLabel *m_forecastLabel;
	QLabel *m_currentTemp;
	QLabel *m_currentHumidity;
	QLabel *m_currentWind;
	QLabel *m_currentSky;
	QLabel *m_sunrise;
	QLabel *m_sunset;
	QLabel *m_currentTempLabel;
	QLabel *m_currentHumidityLabel;
	QLabel *m_currentWindLabel;
	QLabel *m_currentSkyLabel;
	QLabel *m_sunriseLabel;
	QLabel *m_sunsetLabel;
	QLabel *m_clockLabel;
	QLabel *m_localTempLabel;
	QLabel *m_localTemp;
	QLabel *m_localHumidityLabel;
	QLabel *m_localHumidity;
    QLabel *m_currentIcon;
    QLabel *m_lightningLabel;
    
	QVector<QLabel*> m_forecastEntries;
    QVector<QLabel*> m_iconEntries;
    QNetworkAccessManager *m_icon;
    QVector<QString> m_icons;

    WeatherData *m_weatherEvent;
    CalendarData *m_calendarEvent;
    
	int m_calEventsY;
	int m_forecastIndex;
    int m_forecastEntryCount;
	bool m_newEventList;
	bool m_resetForecastTimer;
	double m_humidity;
	double m_temperature;
};

#endif /* __MIRRORFRAME_H__ */
