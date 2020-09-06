#include "MirrorFrame.h"
#include "ui_MirrorFrame.h"

#include "calendar.h"
#include "domoticz/sensor.h"
#include "settingsfactory.h"
#include "WeatherData.h"

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QFont>
#include <QJsonArray>
#include <QJsonObject>
#include <QLayout>
#include <QLayoutItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtGlobal>
#include <QTime>
#include <QTimer>

namespace {

QString epochToTimeOfDay(const quint64 t)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const qint64 timestamp = t*1000 - QDate::currentDate().startOfDay().toMSecsSinceEpoch();
#else
    const qint64 timestamp = t*1000 - QDateTime(QDate::currentDate()).toMSecsSinceEpoch();
#endif
    const QTime s = QTime::fromMSecsSinceStartOfDay(timestamp);
    return s.toString(Qt::DefaultLocaleShortDate);
}

} // anonymous namespace

MirrorFrame::MirrorFrame(QSharedPointer<QNetworkAccessManager> net) :
    QFrame(0),
    ui(new Ui::MirrorFrame),
    m_net(net),
    m_iconCache(net, this),
    m_weatherEvent(0),
    m_forecastIndex(0),
    m_forecastEntryCount(0)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create();
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

    createWeatherSystem();
    createCalendarSystem();

    settings->beginGroup(DOMOTICZ_SETTINGS_GROUP);
    if (settings->contains(DOMOTICZ_INDOOR_TEMP_IDX_SETTINGS_KEY)) {
        bool idxOk;
        const int idx = settings->value(DOMOTICZ_INDOOR_TEMP_IDX_SETTINGS_KEY).toInt(&idxOk);
        if (idxOk && idx > 0 &&
            domoticz::Sensor::Create(m_indoorTempSensor, idx, m_net, this))
        {
            connect(&m_localTempTimer, SIGNAL(timeout()), m_indoorTempSensor, SLOT(update()));
            connect(m_indoorTempSensor, SIGNAL(valueUpdated(const QString&, const QString&, const QDateTime&)),
                    this, SLOT(indoorTemperature(const QString&, const QString&, const QDateTime&)));
            m_localTempTimer.start(TEMPERATURE_TIMEOUT);
            m_indoorTempSensor->update();
        }
    }
    settings->endGroup();
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
    delete m_weatherEvent;
    delete ui;
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
    calendar::Calendar* cal;
    if (calendar::Calendar::Create(cal, m_net, this)) {
        qDebug() << "Successfully created a calendar widget";
    }
}

void MirrorFrame::indoorTemperature(const QString &, const QString &temperature, const QDateTime& updated)
{
    const QDateTime yesterday = QDateTime::currentDateTime().addDays(-1);
    const double humidity = 0.0;

    ui->localTemp->setText(temperature);
    if (updated < yesterday) {
        ui->localTemp->setStyleSheet("QLabel { color : red; }");
    }
    else {
        // reset to default stylesheet
        ui->localTemp->setStyleSheet(QString());
    }
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
    qWarning() << __PRETTY_FUNCTION__ << ":" << error;
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
    const QDate today = QDate::currentDate();
    int humidity;
    double high;
    double low;
    double wind;
    QString sky;

    qint64 secs = jobj["dt"].toInt();
    secs *= 1000;
    dt.setMSecsSinceEpoch(secs);
    if (dt.date() < today)
        return;

    QJsonObject main = jobj["main"].toObject();
    humidity = main["humidity"].toInt();
    high = main["temp_max"].toDouble() + 0.5;
    low = main["temp_min"].toDouble() + 0.5;
    wind = jobj["wind"].toObject()["speed"].toDouble();

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
        QString text = QString("%1 (%2): high: %3%4, low: %5%6: %7")
                .arg(dt.date() == today ? QString("Today's") : dt.toString("dddd"))
                .arg(dt.time().toString(Qt::DefaultLocaleShortDate))
                .arg((int)high)
                .arg(QChar(0260))
                .arg((int)low)
                .arg(QChar(0260))
                .arg(sky);

        // wind speed limits taken from https://sv.wikipedia.org/wiki/Vindstyrka#Vindskalor_f%C3%B6r_vanliga_vindar
        if (wind <= 3.3) {
            text.append(", calm");
        }
        else if (wind <= 8.0) {
            text.append(", breezy");
        }
        else {
            text.append(", windy");
        }
        if (high >= 26) {
            if (humidity > 70) {
                text.append(" and humid");
            }
            else if (humidity < 40 && humidity > 0) {
                text.append(" and dry");
            }
        }
        lb->setText(text);
        m_forecastIndex++;
    }
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