#include "MirrorFrame.h"
#include "ui_MirrorFrame.h"

#include "utils/settingsfactory.h"
#include "widgets/slideshow.h"
#include "widgets/calendar/calendar.h"
#include "widgets/departures/departurewidget.h"
#include "widgets/schedule/schedule.h"
#include "widgets/weather/currentconditions.h"
#include "widgets/weather/forecast.h"
#include "widgets/weather/globe.h"
#include "widgets/sensors/sensors.h"

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QVBoxLayout>

#define WIDGET_CYCLE_INTERVAL (3000)

MirrorFrame::MirrorFrame(QSharedPointer<QNetworkAccessManager> net) :
    QFrame(0),
    ui(new Ui::MirrorFrame),
    m_weatherWidget(0),
    m_slideshow(0),
    m_net(net)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create();
    ui->setupUi(this);
    ui->versionLabel->setText(QString("Version: %1").arg(QString(VERSION_STRING)));

    ui->clock->setDisplayFormat(QLocale().timeFormat(QLocale::LongFormat));
    connect(&m_clockTimer, &QTimer::timeout, this, &MirrorFrame::updateClock);
    m_clockTimer.setTimerType(Qt::CoarseTimer);
    m_clockTimer.start(500);

    connect(&m_cycleTimer, &QTimer::timeout, this, &MirrorFrame::cycleWidgets);
    m_cycleTimer.setTimerType(Qt::VeryCoarseTimer);
    m_cycleTimer.setInterval(WIDGET_CYCLE_INTERVAL);

    createLeftPanel();
    createRightPanel();
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
    delete ui;
}

void MirrorFrame::resizeEvent(QResizeEvent *)
{
    if (m_weatherWidget && m_slideshow) {
        m_slideshow->setMinimumWidth(m_weatherWidget->width());
    }
}

void MirrorFrame::createRightPanel()
{
    m_weatherWidget = new weather::CurrentConditions(m_net, this);
    ui->topRightLayout->insertWidget(ui->topRightLayout->indexOf(ui->readingsWidget), m_weatherWidget);

    sensors::Sensors *sensorWidget;
    if (sensors::Sensors::Create(sensorWidget, m_net, this)) {
        qDebug() << __PRETTY_FUNCTION__ << "adding sensor widget";
        ui->readingsWidget->addWidget(sensorWidget);
    }

    weather::Forecast* forecast = new weather::Forecast(m_net, this);
    ui->readingsWidget->addWidget(forecast);

    if (ui->readingsWidget->count() > 1) {
        m_cycleTimer.start();
    }

    Slideshow *slideshow;
    if (Slideshow::Create(slideshow, this)) {
        qDebug() << "Successfully created a picture slideshow";
        slideshow->setAlignment(Qt::AlignRight);
        ui->bottomRightLayout->insertWidget(1, slideshow, 0, Qt::AlignRight | Qt::AlignBottom);
        m_slideshow = slideshow;
    }
    connect(this, SIGNAL(minuteChanged()), ui->globe, SLOT(repaint()));
}

void MirrorFrame::createLeftPanel()
{
    calendar::Calendar* cal;
    if (calendar::Calendar::Create(cal, m_net, this)) {
        qDebug() << "Successfully created a calendar widget";
        ui->leftVerticalLayout->insertWidget(0, cal, 0, Qt::AlignLeft);
        connect(this, &MirrorFrame::dayChanged, cal, &calendar::Calendar::changeDay);
    }

    departure::DepartureWidget* departures;
    if (departure::DepartureWidget::Create(departures, m_net, this)) {
        qDebug() << "Successfully created a departure widget";
        ui->leftVerticalLayout->addWidget(departures);
    }
    schedule::Schedule* schedule;
    if (schedule::Schedule::Create(schedule, m_net, this)) {
        qDebug() << "Successfully created a schedule widget";
        ui->leftVerticalLayout->addWidget(schedule, 0, Qt::AlignLeft);
    }
}

void MirrorFrame::updateClock()
{
    const QTime time = ui->clock->time();
    const QDate day = ui->clock->date();
    const QDateTime now = QDateTime::currentDateTime();
    const QDate today = now.date();

    ui->clock->setDateTime(now);
    if (now.time().minute() != time.minute()) {
        emit minuteChanged();
    }
    if (today != day) {
        emit dayChanged(today);
    }
}

void MirrorFrame::cycleWidgets()
{
    if (ui->readingsWidget->count() > 1) {
        int idx = ui->readingsWidget->currentIndex();
        idx = (idx + 1) % ui->readingsWidget->count();
        ui->readingsWidget->setCurrentIndex(idx);
    }
}
