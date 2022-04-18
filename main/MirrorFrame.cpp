#include "MirrorFrame.h"
#include "ui_MirrorFrame.h"

#include "utils/settingsfactory.h"
#include "widgets/slideshow.h"
#include "widgets/calendar/calendar.h"
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

MirrorFrame::MirrorFrame(QSharedPointer<QNetworkAccessManager> net) :
    QFrame(0),
    ui(new Ui::MirrorFrame),
    m_calendar(0),
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

    createClimateSystem();
    createCalendarSystem();
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
    if (m_calendar && m_slideshow) {
        m_slideshow->setMinimumWidth(m_calendar->width());
    }
}

void MirrorFrame::createClimateSystem()
{
    QVBoxLayout* layout = new QVBoxLayout;

    weather::CurrentConditions* weather = new weather::CurrentConditions(m_net, this);
    layout->addWidget(weather);

    sensors::Sensors *sensorWidget;
    if (sensors::Sensors::Create(sensorWidget, m_net, this)) {
        qDebug() << __PRETTY_FUNCTION__ << "adding sensor widget";
        layout->addWidget(sensorWidget, 0);
    }

    weather::Forecast* forecast = new weather::Forecast(m_net, this);
    layout->addWidget(forecast, 1);

    ui->topHorizontalLayout->addLayout(layout);

    weather::Globe* globe = new weather::Globe(this);
    ui->rightVerticalLayout->insertWidget(1, globe, 0, Qt::AlignRight);
    connect(this, SIGNAL(minuteChanged()), globe, SLOT(repaint()));
}

void MirrorFrame::createCalendarSystem()
{
    calendar::Calendar* cal;
    if (calendar::Calendar::Create(cal, m_net, this)) {
        qDebug() << "Successfully created a calendar widget";
        ui->topHorizontalLayout->insertWidget(0, cal, 0, Qt::AlignLeft);
        connect(this, &MirrorFrame::dayChanged, cal, &calendar::Calendar::changeDay);
        m_calendar = cal;
    }

    Slideshow *slideshow;
    if (Slideshow::Create(slideshow, this)) {
        qDebug() << "Successfully created a picture slideshow";
        ui->leftVerticalLayout->addWidget(slideshow, 0);
        m_slideshow = slideshow;
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
