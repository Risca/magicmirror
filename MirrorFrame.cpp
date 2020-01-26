#include "MirrorFrame.h"
#include "ui_MirrorFrame.h"

#include "settingsfactory.h"

#include <QDate>
#include <QDebug>
#include <QFont>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QLayout>
#include <QLayoutItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
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
    m_net(net),
    m_iconCache(net, this),
    m_weatherEvent(0),
    m_calendarEvent(0),
    m_forecastIndex(0),
    m_forecastEntryCount(0),
    m_newEventList(false)
{
    QLocale::setDefault(QLocale(SettingsFactory::Create()->value("locale", "en_EN").toString()));
    ui->setupUi(this);
    ui->versionLabel->setText(QString("Version: %1").arg(QString(VERSION_STRING)));

    connect(&m_iconCache, SIGNAL(iconDownloaded(QString)), this, SLOT(iconDownloaded(QString)));

    connect(&m_clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
    m_clockTimer.start(500);

    for (int i = 0; i < 5; i++) {
        QLabel *forecast = new QLabel(this);
        QFont f = forecast->font();
        f.setBold(true);
        f.setPointSize(15);
        forecast->setFont(f);
        ui->forecastLayout->addWidget(forecast, i, 0);

        QLabel *icon = new QLabel(this);
        ui->forecastLayout->addWidget(icon, i, 1);
    }

    setupMqttSubscriber();
    createWeatherSystem();
    createCalendarSystem();

    connect(&m_localTempTimer, SIGNAL(timeout()), this, SLOT(updateLocalTemp()));
    m_localTempTimer.start(1000);        // Get sensor data every second
    updateLocalTemp();
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
    connect(&m_lightningTimer, SIGNAL(timeout()), this, SLOT(lightningTimeout()));
#endif
}

void MirrorFrame::createWeatherSystem()
{
    if (WeatherData::Create(m_weatherEvent, m_net, this)) {
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

        connect(&m_currentWeatherTimer, SIGNAL(timeout()), m_weatherEvent, SLOT(processCurrentWeather()));
        m_currentWeatherTimer.start(CURRENT_TIMEOUT);
        m_weatherEvent->processCurrentWeather();

        connect(&m_forecastTimer, SIGNAL(timeout()), m_weatherEvent, SLOT(processForecast()));
        m_forecastTimer.start(FORECAST_TIMEOUT);
        m_weatherEvent->processForecast();
    }
}

void MirrorFrame::createCalendarSystem()
{
    if (CalendarInterface::Create(m_calendarEvent, m_net, this)) {
        connect(m_calendarEvent, SIGNAL(error(QString)), this, SLOT(calendarEventsError(QString)));
        connect(m_calendarEvent, SIGNAL(newEvent(QString)), this, SLOT(calendarEventsEvent(QString)));
        connect(m_calendarEvent, SIGNAL(finished()), this, SLOT(calendarEventsDone()));

        connect(&m_calendarTimer, SIGNAL(timeout()), m_calendarEvent, SLOT(sync()));
        m_calendarTimer.start(CALEVENTS_TIMEOUT);
        m_calendarEvent->sync();
    }
}

void MirrorFrame::registerTouchEvent()
{
    emit touchDetected();
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
    m_currentIconId = id;
    if (!m_iconCache.exists(id)) {
        m_iconCache.download(id);
    }
    else {
        QImage image;
        if (m_iconCache.get(id, image)) {
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
    m_forecastIcons.clear();
}

void MirrorFrame::forecastEntry(const QJsonObject &jobj)
{
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
        QString icon = obj["icon"].toString();
        if (!m_iconCache.exists(icon)) {
            m_iconCache.download(icon);
        }
        else {
            QImage image;
            if (m_iconCache.get(icon, image)) {
                QPixmap pixmap;
                pixmap.convertFromImage(image);
                QLabel *lb = static_cast<QLabel*>(ui->forecastLayout->itemAtPosition(m_forecastIndex, 1)->widget());
                lb->setPixmap(pixmap);
            }
        }
        m_forecastIcons.push_front(icon);
    }

    if (m_forecastIndex < ui->forecastLayout->rowCount()) {
        QLabel *lb = static_cast<QLabel*>(ui->forecastLayout->itemAtPosition(m_forecastIndex, 0)->widget());
        if (now.date() == dt.date()) {
            QString text = QString("Today's (%1) high: %2%3, low: %4%5, %6")
                    .arg(dt.time().toString(Qt::DefaultLocaleShortDate))
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
            QString text = QString("%1 (%2): high: %3%4, low: %5%6: %7")
                    .arg(dt.toString("dddd"))
                    .arg(dt.time().toString(Qt::DefaultLocaleShortDate))
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
        m_forecastIndex++;
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

void MirrorFrame::iconDownloaded(const QString& icon)
{
    if (icon == m_currentIconId) {
        currentIcon(m_currentIconId);
    }

    for (int i = 0; i < m_forecastIcons.size(); i++) {
        if (icon == m_forecastIcons[i]) {
            QLabel *lb = static_cast<QLabel*>(ui->forecastLayout->itemAtPosition(i, 1)->widget());
            QImage image;
            if (m_iconCache.get(icon, image)) {
                QPixmap pixmap;
                pixmap.convertFromImage(image);
                lb->setPixmap(pixmap);
            }
        }
    }
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
    m_lightningTimer.stop();
    m_lightningTimer.setInterval(THIRTY_MINUTES);
    m_lightningTimer.start();
    emit touchDetected();

}

void MirrorFrame::lightningTimeout()
{
    ui->lightningLabel->setText("");
    m_lightningTimer.stop();
}
