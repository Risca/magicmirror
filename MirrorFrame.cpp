#include "MirrorFrame.h"
#include "ui_MirrorFrame.h"

#include "settingsfactory.h"

#include <QDate>
#include <QDebug>
#include <QFont>
#include <QJsonArray>
#include <QJsonObject>
#include <QLayout>
#include <QLayoutItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QStateMachine>
#include <QtGlobal>
#include <QTime>
#include <QTimer>

namespace {

QString epochToTimeOfDay(const quint64 t)
{
    const qint64 timestamp = t*1000 - QDateTime(QDate::currentDate()).toMSecsSinceEpoch();
    const QTime s = QTime::fromMSecsSinceStartOfDay(timestamp);
    return s.toString(Qt::DefaultLocaleShortDate);
}

void clearLayout(QLayout *layout) {
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
           delete item->widget();
        }
        delete item;
    }
}

} // anonymous namespace

MirrorFrame::MirrorFrame(QSharedPointer<QNetworkAccessManager> net) :
    QFrame(0),
    ui(new Ui::MirrorFrame),
    m_icon(net),
    m_weatherEvent(0),
    m_forecastIndex(0),
    m_forecastEntryCount(0),
    m_newEventList(false)
{
    QLocale::setDefault(QLocale(SettingsFactory::Create()->value("locale", "en_EN").toString()));
    ui->setupUi(this);

    m_calendarTimer = new QTimer();
    m_forecastTimer = new QTimer();
    m_currentWeatherTimer = new QTimer();
    m_clockTimer = new QTimer();
    m_localTempTimer = new QTimer();
    m_monitorTimer = new QTimer();
    m_monitorState = new QStateMachine(this);

    QFont f("Roboto");
    f.setPixelSize(50*2/3);
    f.setBold(true);
    ui->calLabel->setFont(f);
    ui->lightningLabel->setFont(f);
    ui->currentLabel->setFont(f);
    ui->forecastLabel->setFont(f);

    f.setPixelSize(40*2/3);
    ui->clockLabel->setFont(f);
    connect(m_clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
    m_clockTimer->start(500);

    f.setPixelSize(30*2/3);

    ui->currentTempLabel->setFont(f);
    ui->localTempLabel->setFont(f);
    ui->currentHumidityLabel->setFont(f);
    ui->localHumidityLabel->setFont(f);
    ui->currentWindLabel->setFont(f);
    ui->currentSkyLabel->setFont(f);
    ui->sunriseLabel->setFont(f);
    ui->sunsetLabel->setFont(f);

    f.setPixelSize(25*2/3);
    f.setBold(false);

    ui->localTemp->setFont(f);
    ui->localHumidity->setFont(f);
    ui->currentTemp->setFont(f);
    ui->currentHumidity->setFont(f);
    ui->currentTemp->setFont(f);
    ui->currentHumidity->setFont(f);
    ui->sunrise->setFont(f);
    ui->currentWind->setFont(f);
    ui->currentSky->setFont(f);
    ui->sunset->setFont(f);

    for (int i = 0; i < 5; i++) {
        QLabel *forecast = new QLabel(this);
        forecast->setFont(f);
        ui->forecastLayout->addWidget(forecast, i, 0);
        m_forecastEntries.push_back(forecast);

        QLabel *icon = new QLabel(this);
        ui->forecastLayout->addWidget(icon, i, 1);
        m_iconEntries.push_back(icon);
    }

    connect(m_icon.get(), SIGNAL(finished(QNetworkReply*)), this, SLOT(iconReplyFinished(QNetworkReply*)));

    setupMqttSubscriber();
    createWeatherSystem();
    createCalendarSystem();
    createStateMachine();

    connect(m_localTempTimer, SIGNAL(timeout()), this, SLOT(updateLocalTemp()));
    m_localTempTimer->start(1000);        // Get sensor data every second
    updateLocalTemp();

    const int monitorTimeout = SettingsFactory::Create()->value("screentimeout", MONITOR_TIMEOUT / (1000 * 60)).toInt() * 1000 * 60;
    qDebug() << __PRETTY_FUNCTION__ << ": setting monitor timeout to" << monitorTimeout;
    m_monitorTimer->start(monitorTimeout);
}

MirrorFrame *MirrorFrame::Create()
{
    QSharedPointer<QNetworkAccessManager> net(new QNetworkAccessManager);
    if (net) {
        return new MirrorFrame(net);
    }
    return 0;
}

MirrorFrame::~MirrorFrame()
{
}

void MirrorFrame::setupMqttSubscriber()
{
#if 0
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Mqtt");
    QString hostname = settings->value("server").toString();
    QHostInfo lookup = QHostInfo::fromName(hostname);
    QList<QHostAddress> addresses = lookup.addresses();

    if (addresses.size() > 0) {
        m_mqttClient = new QMqttSubscriber(addresses.at(0), settings->value("port").toInt(), this);
        qDebug() << __PRETTY_FUNCTION__ << ": setting host address to" << addresses.at(0);
    }
    else {
        m_mqttClient = new QMqttSubscriber(QHostAddress::LocalHost, settings->value("port").toInt(), this);
        qDebug() << __PRETTY_FUNCTION__ << ": Using localhost";
    }
    connect(m_mqttClient, SIGNAL(connectionComplete()), this, SLOT(connectionComplete()));
    connect(m_mqttClient, SIGNAL(disconnectedEvent()), this, SLOT(disconnectedEvent()));
    connect(m_mqttClient, SIGNAL(messageReceivedOnTopic(QString, QString)), this, SLOT(messageReceivedOnTopic(QString, QString)));
    ui->lightningLabel->setText("Connecting to MQTT server...");
    m_mqttClient->connectToHost();
    m_lightningTimer = new QTimer();
    connect(m_lightningTimer, SIGNAL(timeout()), this, SLOT(lightningTimeout()));
#endif
}

void MirrorFrame::createWeatherSystem()
{
    if (WeatherData::Create(m_weatherEvent, this)) {
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

        connect(m_currentWeatherTimer, SIGNAL(timeout()), m_weatherEvent, SLOT(processCurrentWeather()));
        m_currentWeatherTimer->start(CURRENT_TIMEOUT);
        m_weatherEvent->processCurrentWeather();

        connect(m_forecastTimer, SIGNAL(timeout()), m_weatherEvent, SLOT(processForecast()));
        m_forecastTimer->start(FORECAST_TIMEOUT);
        m_weatherEvent->processForecast();
    }
}

void MirrorFrame::createCalendarSystem()
{
    m_calendarEvent = new CalendarData();
    if (m_calendarEvent) {
        connect(m_calendarEvent, SIGNAL(error(QString)), this, SLOT(calendarEventsError(QString)));
        connect(m_calendarEvent, SIGNAL(newEvent(QString)), this, SLOT(calendarEventsEvent(QString)));
        connect(m_calendarEvent, SIGNAL(finished()), this, SLOT(calendarEventsDone()));

        connect(m_calendarTimer, SIGNAL(timeout()), m_calendarEvent, SLOT(process()));
        m_calendarTimer->start(CALEVENTS_TIMEOUT);
        m_calendarEvent->process();
    }
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

void MirrorFrame::updateLocalTemp()
{
    double temperature = 0.0;
    double humidity = 0.0;

#ifdef __USE_RPI__
    if (getValues(&temperature, &humidity) == 0) {
        qDebug() << __PRETTY_FUNCTION__ << ": temp: " << temperature << ", humidity:" << humidity;
    }
#endif

    ui->localTemp->setText(QString("%1%2").arg(temperature, 0, 'f', 1).arg(QChar(0260)));
    ui->localHumidity->setText(QString("%1%").arg(humidity, 0, 'f', 1));
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
    ui->clockLabel->setText(now.toString(Qt::DefaultLocaleShortDate));
}

void MirrorFrame::sunrise(qint64 t)
{
    ui->sunrise->setText(QString("%1").arg(epochToTimeOfDay(t)));
}

void MirrorFrame::sunset(qint64 t)
{
    ui->sunset->setText(QString("%1").arg(epochToTimeOfDay(t)));
}

void MirrorFrame::weatherEventsDone()
{
    qDebug() << __PRETTY_FUNCTION__;
}

void MirrorFrame::currentIcon(const QString &id)
{
    WeatherIcon icon;

    if (!icon.exists(id)) {
        getIcon(id);
    }
    else {
        QImage image;
        if (icon.get(id, image)) {
            QPixmap pixmap;
            pixmap.convertFromImage(image.scaledToHeight(100, Qt::SmoothTransformation));
            ui->currentIcon->setPixmap(pixmap);
        }
    }
}

void MirrorFrame::currentTemperature(double temp)
{
    ui->currentTemp->setText(QString("%1%2").arg(temp, 0, 'f', 1).arg(QChar(0260)));
}

void MirrorFrame::currentSkyConditions(const QString &sky)
{
    ui->currentSky->setText(QString("%1").arg(sky));
}

void MirrorFrame::currentHumidity(double humidity)
{
    ui->currentHumidity->setText(QString("%1%").arg(humidity));
}

void MirrorFrame::currentWindSpeed(double speed)
{
    speed = speed + 0.5;
    int rounded = (int)speed;

    ui->currentWind->setText(QString("%1 m/s").arg(rounded));
}

void MirrorFrame::weatherDataError(const QString &error)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << error;
}

void MirrorFrame::forecastEntryCount(int c)
{
    m_forecastEntryCount = c;
    m_forecastIndex = 0;
    m_icons.clear();
}

void MirrorFrame::forecastEntry(const QJsonObject &jobj)
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
            if (icon.get(j, image)) {
                QPixmap pixmap;
                pixmap.convertFromImage(image);
                QLabel *lb = m_iconEntries[m_forecastIndex];
                lb->setPixmap(pixmap);
            }
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

void MirrorFrame::calendarEventsError(const QString& error)
{
    qDebug() << __PRETTY_FUNCTION__ << ":" << error;
}

void MirrorFrame::calendarEventsDone()
{
    m_newEventList = true;
}

void MirrorFrame::calendarEventsEvent(const QString &s)
{
    if (m_newEventList)
        deleteCalendarEventsList();

    QLabel *lb = new QLabel(s, this);
    QFont f("Roboto");
    f.setPixelSize(25*2/3);
    f.setBold(false);
    lb->setFont(f);
    ui->calendarLayout->addWidget(lb);
}

void MirrorFrame::deleteCalendarEventsList()
{
    clearLayout(ui->calendarLayout);
    m_newEventList = false;
}

void MirrorFrame::getIcon(const QString &icon)
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
                if (icon.get(i, image)) {
                    QPixmap pixmap;
                    pixmap.convertFromImage(image);
                    lb->setPixmap(pixmap);
                }
            }
        }
    }
    reply->deleteLater();
}

void MirrorFrame::connectionComplete()
{
    ui->lightningLabel->clear();
}

void MirrorFrame::disconnectedEvent()
{
    ui->lightningLabel->setText("Connecting to MQTT server...");
}

void MirrorFrame::messageReceivedOnTopic(const QString &t, const QString &p)
{
    qDebug() << __PRETTY_FUNCTION__ << ": Topic:" << t << ", payload: " << p;
    double d = p.toDouble();
    double distance = d * .621;

    ui->lightningLabel->setText(QString("Lightning detected at %1 miles").arg(distance, 0, 'f', 1));
    m_lightningTimer->stop();
    m_lightningTimer->setInterval(THIRTY_MINUTES);
    m_lightningTimer->start();
    emit touchDetected();

}

void MirrorFrame::lightningTimeout()
{
    ui->lightningLabel->setText("");
    m_lightningTimer->stop();
}
