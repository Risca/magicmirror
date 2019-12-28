#include "MirrorFrame.h"

MirrorFrame::MirrorFrame(QFrame *parent) : QFrame(parent)
{

	m_calendarTimer = new QTimer();
	m_forecastTimer = new QTimer();
	m_currentWeatherTimer = new QTimer();
	m_clockTimer = new QTimer();
	m_localTempTimer = new QTimer();
	m_monitorTimer = new QTimer();
	m_monitorState = new QStateMachine(this);
	
	QPalette pal(QColor(0,0,0));
	setBackgroundRole(QPalette::Window);
	pal.setColor(QPalette::Window, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);

	m_calLabel = new QLabel("<font color='white'>Upcoming Events", this);
	m_forecastLabel = new QLabel("<font color='white'>Local Forecast</font>", this);
	m_currentLabel = new QLabel("<font color='white'>Current Conditions</font>", this);
	m_clockLabel = new QLabel(this);
    m_lightningLabel =  new QLabel(this);

	QFont f("Roboto");
	f.setPixelSize(50);
	f.setBold(true);
	m_calLabel->setGeometry(50, 10, 500, 100);
	m_calLabel->setFont(f);
    m_lightningLabel->setGeometry(50, 800, 1000, 100);
    m_lightningLabel->setFont(f);
	m_currentLabel->setGeometry(50, 900, 600, 100);
	m_currentLabel->setFont(f);
	m_forecastLabel->setGeometry(50, 1400, 500, 100);
	m_forecastLabel->setFont(f);
    
	m_clockLabel->setGeometry(700, 10, 650, 100);
	f.setPixelSize(40);
	m_clockLabel->setFont(f);

	f.setPixelSize(30);
	m_currentTempLabel = new QLabel(this);
	m_currentHumidityLabel = new QLabel(this);
	m_currentWindLabel = new QLabel(this);
	m_currentSkyLabel = new QLabel(this);
	m_localTempLabel = new QLabel(this);
	m_localHumidityLabel = new QLabel(this);

	m_localTempLabel->setGeometry(50, 1000, 350, 50);
	m_localHumidityLabel->setGeometry(400, 1000, 350, 50);
	m_localTempLabel->setFont(f);
	m_localHumidityLabel->setFont(f);
	m_localTempLabel->setText("<center>Inside Temperature</center>");
	m_localHumidityLabel->setText("<center>Inside Humidity</center>");

	m_currentTempLabel = new QLabel(this);
	m_currentTempLabel->setGeometry(50, 1100, 350, 50);
	m_currentTempLabel->setFont(f);
	m_currentTempLabel->setText("<center>Outside Temperature</center>");

	m_currentHumidityLabel = new QLabel(this);
	m_currentHumidityLabel->setGeometry(400, 1100, 350, 50);
	m_currentHumidityLabel->setFont(f);
	m_currentHumidityLabel->setText("<center>Outside Humidity</center>");

	m_sunriseLabel = new QLabel(this);
	m_sunriseLabel->setGeometry(750, 1000, 350, 50);
	m_sunriseLabel->setFont(f);
	m_sunriseLabel->setText("<center>Sunrise</center>");

	m_currentWindLabel = new QLabel(this);
	m_currentWindLabel->setGeometry(50, 1200, 350, 50);
	m_currentWindLabel->setFont(f);
	m_currentWindLabel->setText("<center>Wind Speed</center>");

	m_currentSkyLabel = new QLabel(this);
	m_currentSkyLabel->setGeometry(400, 1200, 350, 50);
	m_currentSkyLabel->setFont(f);
	m_currentSkyLabel->setText("<center>Sky Conditions</center>");

	m_sunsetLabel = new QLabel(this);
	m_sunsetLabel->setGeometry(750, 1100, 350, 50);
	m_sunsetLabel->setFont(f);
	m_sunsetLabel->setText("<center>Sunset</center>");

    m_currentIcon = new QLabel(this);
    m_currentIcon->setGeometry(750, 1200, 350, 100);
    m_currentIcon->setAlignment(Qt::AlignCenter);

	f.setPixelSize(25);
	f.setBold(false);

	m_localTemp = new QLabel(this);
	m_localTemp->setGeometry(50, 1050, 350, 50);
	m_localTemp->setFont(f);

	m_localHumidity = new QLabel(this);
	m_localHumidity->setGeometry(400, 1050, 350, 50);
	m_localHumidity->setFont(f);

	m_currentTemp = new QLabel(this);
	m_currentTemp->setGeometry(50, 1150, 350, 50);
	m_currentTemp->setFont(f);

	m_currentHumidity = new QLabel(this);
	m_currentHumidity->setGeometry(400, 1150, 350, 50);
	m_currentHumidity->setFont(f);

	m_sunrise = new QLabel(this);
	m_sunrise->setGeometry(750, 1050, 350, 50);
	m_sunrise->setFont(f);

	m_currentWind = new QLabel(this);
	m_currentWind->setGeometry(50, 1250, 350, 50);
	m_currentWind->setFont(f);

	m_currentSky = new QLabel(this);
	m_currentSky->setGeometry(400, 1250, 350, 50);
	m_currentSky->setFont(f);

	m_sunset = new QLabel(this);
	m_sunset->setGeometry(750, 1150, 350, 50);
	m_sunset->setFont(f);

	for (int i = 0; i < 5; i++) {
		QLabel *forecast = new QLabel(this);
        QLabel *icon = new QLabel(this);
        icon->setGeometry(50, ((i * 50) + 1500), 100, 50);
		forecast->setGeometry(150, ((i * 50) + 1500), 1150, 50);
		forecast->setFont(f);
		m_forecastEntries.push_back(forecast);
        m_iconEntries.push_back(icon);
	}

    m_icon = new QNetworkAccessManager(this);
    connect(m_icon, SIGNAL(finished(QNetworkReply*)), this, SLOT(iconReplyFinished(QNetworkReply*)));

	m_calEventsY = 110;
	m_forecastIndex = 0;
	m_newEventList = false;
	m_resetForecastTimer = true;
    m_forecastEntryCount = 0;
    m_temperature = 0.0;
    m_humidity = 0.0;
	
    setupMqttSubscriber();
    createWeatherSystem();
    createCalendarSystem();
	createStateMachine();
	enableTimers();

	updateLocalTemp();
}

MirrorFrame::~MirrorFrame()
{
}

void MirrorFrame::setupMqttSubscriber()
{
#if 0
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MagicMirror", "MagicMirror");
    QString hostname = settings.value("mqttserver").toString();
    QHostInfo lookup = QHostInfo::fromName(hostname);
    QList<QHostAddress> addresses = lookup.addresses();
    
    if (addresses.size() > 0) {
        m_mqttClient = new QMqttSubscriber(addresses.at(0), settings.value("mqttport").toInt(), this);
        qDebug() << __PRETTY_FUNCTION__ << ": setting host address to" << addresses.at(0);
    }
    else {
        m_mqttClient = new QMqttSubscriber(QHostAddress::LocalHost, settings.value("mqttport").toInt(), this);
        qDebug() << __PRETTY_FUNCTION__ << ": Using localhost";
    }
    connect(m_mqttClient, SIGNAL(connectionComplete()), this, SLOT(connectionComplete()));
    connect(m_mqttClient, SIGNAL(disconnectedEvent()), this, SLOT(disconnectedEvent()));
    connect(m_mqttClient, SIGNAL(messageReceivedOnTopic(QString, QString)), this, SLOT(messageReceivedOnTopic(QString, QString)));
    m_lightningLabel->setText("Connecting to MQTT server...");
    m_mqttClient->connectToHost();
    m_lightningTimer = new QTimer();
    connect(m_lightningTimer, SIGNAL(timeout()), this, SLOT(lightningTimeout()));
#endif
}

void MirrorFrame::createWeatherSystem()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MagicMirror", "MagicMirror");
    m_weatherEvent = new WeatherData();
    m_weatherEvent->addZip(settings.value("zip").toString(), settings.value("country").toString());
	m_weatherEvent->addAppID(settings.value("appid").toString());

    connect(m_weatherEvent, SIGNAL(temperature(double)), this, SLOT(currentTemperature(double)));
	connect(m_weatherEvent, SIGNAL(humidity(double)), this, SLOT(currentHumidity(double)));
	connect(m_weatherEvent, SIGNAL(windSpeed(double)), this, SLOT(currentWindSpeed(double)));
	connect(m_weatherEvent, SIGNAL(skyConditions(QString)), this, SLOT(currentSkyConditions(QString)));
	connect(m_weatherEvent, SIGNAL(sunrise(qint64)), this, SLOT(sunrise(qint64)));
	connect(m_weatherEvent, SIGNAL(sunset(qint64)), this, SLOT(sunset(qint64)));
    connect(m_weatherEvent, SIGNAL(forecastEntryCount(int)), this, SLOT(forecastEntryCount(int)));
	connect(m_weatherEvent, SIGNAL(finished()), this, SLOT(weatherEventsDone()));
	connect(m_weatherEvent, SIGNAL(error(QString)), this, SLOT(weatherDataError(QString)));
    connect(m_weatherEvent, SIGNAL(forecastEntry(QJsonObject)), this, SLOT(forecastEntry(QJsonObject)));
    connect(m_weatherEvent, SIGNAL(currentIcon(QString)), this, SLOT(currentIcon(QString)));
}

void MirrorFrame::createCalendarSystem()
{
	m_calendarEvent = new CalendarData();
	connect(m_calendarEvent, SIGNAL(error(QString)), this, SLOT(calendarEventsError(QString)));
	connect(m_calendarEvent, SIGNAL(newEvent(QString)), this, SLOT(calendarEventsEvent(QString)));
	connect(m_calendarEvent, SIGNAL(finished()), this, SLOT(calendarEventsDone()));
}

void MirrorFrame::registerTouchEvent()
{
	emit touchDetected();
}

void MirrorFrame::createStateMachine()
{
	QState *on = new QState();
	QState *off = new QState();
	
	off->addTransition(this, SIGNAL(touchDetected()), on);
	on->addTransition(m_monitorTimer, SIGNAL(timeout()), off);
	connect(on, SIGNAL(entered()), this, SLOT(monitorOn()));
	connect(off, SIGNAL(entered()), this, SLOT(monitorOff()));
	connect(this, SIGNAL(touchDetected()), this, SLOT(resetMonitorTimer()));
	m_monitorState->addState(on);
	m_monitorState->addState(off);
	m_monitorState->setInitialState(on);
	m_monitorState->start();
}

void MirrorFrame::enableTimers()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "MagicMirror", "MagicMirror");
	int monitorTimeout = settings.value("screentimeout").toInt();
	QDateTime now = QDateTime::currentDateTime();
	QDateTime midnight(QDate(now.date().year(), now.date().month(), now.date().day()), QTime(4, 0, 0));
    midnight = midnight.addDays(1);
		
	connect(m_calendarTimer, SIGNAL(timeout()), this, SLOT(getEvents()));
	m_calendarTimer->start(CALEVENTS_TIMEOUT);		// Get Events once an hour
	
	connect(m_forecastTimer, SIGNAL(timeout()), this, SLOT(getForecast()));
	m_forecastTimer->start(FORECAST_TIMEOUT);		// set to timeout at midnight, we reset it to 12 hours later
	
	connect(m_currentWeatherTimer, SIGNAL(timeout()), this, SLOT(getCurrentWeather()));
	m_currentWeatherTimer->start(CURRENT_TIMEOUT);	// get current weather conditions once an hour
	
	connect(m_clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
	m_clockTimer->start(500);					// Update the clock 2x a second

	connect(m_localTempTimer, SIGNAL(timeout()), this, SLOT(updateLocalTemp()));
	m_localTempTimer->start(1000);        // Get sensor data every second

	if (monitorTimeout == 0)
		monitorTimeout = MONITOR_TIMEOUT;
	else
		monitorTimeout = monitorTimeout * 1000 * 60;

	qDebug() << __PRETTY_FUNCTION__ << ": setting monitor timeout to" << monitorTimeout;
    qDebug() << __PRETTY_FUNCTION__ << ": Getting next forecast at" << midnight;
	m_monitorTimer->start(monitorTimeout);
}

void MirrorFrame::updateLocalTemp()
{
    double t = 0.0;
    double h = 0.0;
    
#ifdef __USE_RPI__
	if (getValues(&t, &h) == 0) {
		m_temperature = t;
		m_humidity = h;
	}
	qDebug() << __PRETTY_FUNCTION__ << ": temp: " << m_temperature << ", humidity:" << m_humidity;
#endif
	m_localTemp->setText(QString("<center>%1%2</center>").arg(m_temperature, 0, 'f', 1).arg(QChar(0260)));
	m_localHumidity->setText(QString("<center>%1%</center>").arg(m_humidity, 0, 'f', 1));
}

void MirrorFrame::resetMonitorTimer()
{
	if (m_monitorTimer->isActive())
		m_monitorTimer->stop();

	m_monitorTimer->start(MONITOR_TIMEOUT);
}

void MirrorFrame::monitorOff()
{
	turnMonitorOff();
	if (m_monitorTimer->isActive())
		m_monitorTimer->stop();
}

void MirrorFrame::monitorOn()
{
	if (!m_monitorTimer->isActive()) {
		m_monitorTimer->setInterval(MONITOR_TIMEOUT);
		m_monitorTimer->start();
	}
	turnMonitorOn();
}

void MirrorFrame::turnMonitorOff()
{
	QProcess p;
	
	p.start("vcgencmd", QStringList() << "display_power" << "0");
	p.waitForFinished();
}

void MirrorFrame::turnMonitorOn()
{
	QProcess p;
	
	p.start("vcgencmd", QStringList() << "display_power" << "1");
	p.waitForFinished();
}

void MirrorFrame::updateClock()
{
	QDateTime now = QDateTime::currentDateTime();
	m_clockLabel->setText(now.toString("ddd MMM d h:mm ap"));
}

void MirrorFrame::getCurrentWeather()
{
    m_weatherEvent->processCurrentWeather();
}

void MirrorFrame::getForecast()
{
    m_weatherEvent->processForecast();
}

void MirrorFrame::sunrise(qint64 t)
{
	QDateTime s;
	s.setMSecsSinceEpoch(t * 1000);
	m_sunrise->setText(QString("<center>%1</center>").arg(s.toString("hh:mm ap")));
}

void MirrorFrame::sunset(qint64 t)
{
	QDateTime s;
	s.setMSecsSinceEpoch(t * 1000);
	m_sunset->setText(QString("<center>%1</center>").arg(s.toString("hh:mm ap")));
}

void MirrorFrame::weatherEventsDone()
{
	qDebug() << __PRETTY_FUNCTION__;
}

void MirrorFrame::currentIcon(QString id)
{
    WeatherIcon icon;
    
    if (!icon.exists(id)) {
        getIcon(id);
    }
    else {
        QImage image;
        QPixmap pixmap;
        icon.get(id, &image);
        pixmap.convertFromImage(image.scaledToHeight(100, Qt::SmoothTransformation));
        m_currentIcon->setPixmap(pixmap);
    }
}

void MirrorFrame::currentTemperature(double temp)
{
	m_currentTemp->setText(QString("<center>%1%2</center>").arg(temp, 0, 'f', 1).arg(QChar(0260)));
}

void MirrorFrame::currentSkyConditions(QString sky)
{
	m_currentSky->setText(QString("<center>%1</center>").arg(sky));
}

void MirrorFrame::currentHumidity(double humidity)
{
	m_currentHumidity->setText(QString("<center>%1%</center>").arg(humidity));
}

void MirrorFrame::currentWindSpeed(double speed)
{
	speed = speed + 0.5;
	int rounded = (int)speed;

	m_currentWind->setText(QString("<center>%1 mph</center>").arg(rounded));
}

void MirrorFrame::weatherDataError(QString)
{
	qDebug() << __PRETTY_FUNCTION__;
}

void MirrorFrame::forecastEntryCount(int c)
{
    m_forecastEntryCount = c;
    m_forecastIndex = 0;
    m_icons.clear();
}

void MirrorFrame::forecastEntry(QJsonObject jobj)
{
    WeatherIcon icon;
    QDateTime dt;
    QDateTime now = QDateTime::currentDateTime();
    int humidity;
    double high;
    double low;
    double wind;
    QString sky;

    qint64 secs = jobj["dt"].toInt();
    secs *= 1000;
    dt.setMSecsSinceEpoch(secs);
    if (dt.date() < now.date())
        return;
    
    humidity = jobj["humidity"].toInt();
    QJsonObject temp = jobj["temp"].toObject();
    high = temp["max"].toDouble() + 0.5;
    low = temp["min"].toDouble() + 0.5;
    wind = jobj["speed"].toDouble();
    
    QJsonArray weather = jobj["weather"].toArray();
    for (int i = 0; i < weather.size(); ++i) {
        QJsonObject obj = weather[i].toObject();
        sky = obj["main"].toString();
        QString j = obj["icon"].toString();
        if (!icon.exists(j)) {
            getIcon(j);
        }
        else {
            QImage image;
            QPixmap pixmap;
            icon.get(j, &image);
            pixmap.convertFromImage(image);
            QLabel *lb = m_iconEntries[m_forecastIndex];
            lb->setPixmap(pixmap);
        }
        m_icons.push_front(j);
    }

    if (m_forecastIndex < m_forecastEntries.size()) {
        QLabel *lb = m_forecastEntries[m_forecastIndex++];
        if (now.date() == dt.date()) {
            QString text = QString("Today's high: %1%2, low: %3%4, %5")
                .arg((int)high)
                .arg(QChar(0260))
                .arg((int)low)
                .arg(QChar(0260))
                .arg(sky);

            if (wind <= 5.0) {
                text.append(", calm");
            }
            else if (wind <= 15.0) {
                text.append(", breezy");
            }
            else {
                text.append(", windy");
            }
            if (high >= 80) {
                if (humidity > 70) {
                    text.append(" and humid");
                }
                else if (humidity < 40 && humidity > 0) {
                    text.append(" and dry");
                }
            }
            lb->setText(text);
        }
        else {
            QString text = QString("%1: high: %2%3, low: %4%5: %6")
                .arg(dt.toString("dddd"))
                .arg((int)high)
                .arg(QChar(0260))
                .arg((int)low)
                .arg(QChar(0260))
                .arg(sky);

            if (wind <= 5.0) {
                text.append(", calm");
            }
            else if (wind <= 15.0) {
                text.append(", breezy");
            }
            else {
                text.append(", windy");
            }
            if (high >= 80) {
                if (humidity > 70) {
                    text.append(" and humid");
                }
                else if (humidity < 40 && humidity > 0) {
                    text.append(" and dry");
                }
            }
            lb->setText(text);
        }
    }
}

void MirrorFrame::getEvents()
{
    m_calendarEvent->process();
}

void MirrorFrame::calendarEventsError(QString)
{
	qDebug() << __PRETTY_FUNCTION__ << ": Error trying to talk to the calendar server";
}

void MirrorFrame::calendarEventsDone()
{
	m_newEventList = true;
}

void MirrorFrame::calendarEventsEvent(QString s)
{
	if (m_newEventList)
		deleteCalendarEventsList();

	QLabel *lb = new QLabel(s, this);
	QFont f("Roboto");
	f.setPixelSize(25);
	lb->setFont(f);
	lb->setGeometry(100, m_calEventsY + 50, 1000, 50);
	lb->show();
	m_calEventsY += 50;
	m_calendarEvents.push_back(lb);
}

void MirrorFrame::deleteCalendarEventsList()
{
    for (int i = 0; i < m_calendarEvents.size(); i++) {
        QLabel *lb = m_calendarEvents.at(i);
        lb->hide();
        delete lb;
    }
    m_calendarEvents.clear();
    m_newEventList = false;
    m_calEventsY = 110;
}

void MirrorFrame::getIcon(QString icon)
{
    WeatherIcon i;
    
    if (!i.exists(icon) && icon.size() > 0) {
        QUrl u("http://openweathermap.org/img/w/" + icon + ".png");
        m_icon->get(QNetworkRequest(u));
    }
}

void MirrorFrame::iconReplyFinished(QNetworkReply *reply)
{
    WeatherIcon icon;
    if (reply->error()) {
		qWarning() << __PRETTY_FUNCTION__ << ":" << reply->errorString();
	}
	else {
        QNetworkRequest r = reply->request();
        QString i = r.url().fileName();
        if (!icon.exists(i) && i.length() > 0) {
            icon.store(i, reply->readAll());
        }
        for (int j = 0; j < m_icons.size(); j++) {
            if (i.contains(m_icons[j])) {
                QLabel *lb = m_iconEntries[j];
                QImage image;
                QPixmap pixmap;
                icon.get(i, &image);
                pixmap.convertFromImage(image);
                lb->setPixmap(pixmap);
            }
        }
    }
    reply->deleteLater();
}

void MirrorFrame::connectionComplete()
{
    m_lightningLabel->clear();
}

void MirrorFrame::disconnectedEvent()
{
    m_lightningLabel->setText("Connecting to MQTT server...");
}

void MirrorFrame::messageReceivedOnTopic(QString t, QString p)
{
    qDebug() << __PRETTY_FUNCTION__ << ": Topic:" << t << ", payload: " << p;
    double d = p.toDouble();
    double distance = d * .621;
    
    m_lightningLabel->setText(QString("Lightning detected at %1 miles").arg(distance, 0, 'f', 1));
    m_lightningTimer->stop();
    m_lightningTimer->setInterval(THIRTY_MINUTES);
    m_lightningTimer->start();
    emit touchDetected();
        
}

void MirrorFrame::lightningTimeout()
{
    m_lightningLabel->setText("");
    m_lightningTimer->stop();
}
