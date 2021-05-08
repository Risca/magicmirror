#include "MirrorFrame.h"
#include "ui_MirrorFrame.h"

#include "utils/settingsfactory.h"
#include "widgets/calendar/calendar.h"
#include "widgets/weather/currentconditions.h"
#include "widgets/weather/forecast.h"
#include "widgets/sensors/sensors.h"

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QVBoxLayout>

MirrorFrame::MirrorFrame(QSharedPointer<QNetworkAccessManager> net) :
    QFrame(0),
    ui(new Ui::MirrorFrame),
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

void MirrorFrame::createClimateSystem()
{
    QVBoxLayout* layout = new QVBoxLayout;

    weather::CurrentConditions* weather = new weather::CurrentConditions(m_net, this);
    layout->addWidget(weather, 0);

    sensors::Sensors *sensorWidget;
    if (sensors::Sensors::Create(sensorWidget, m_net, this)) {
        qDebug() << __PRETTY_FUNCTION__ << "adding sensor widget";
        layout->addWidget(sensorWidget, 0);
    }

    weather::Forecast* forecast = new weather::Forecast(m_net, this);
    layout->addWidget(forecast, 1, Qt::AlignRight);

    ui->topHorizontalLayout->addLayout(layout);
}

void MirrorFrame::createCalendarSystem()
{
    calendar::Calendar* cal;
    if (calendar::Calendar::Create(cal, m_net, this)) {
        qDebug() << "Successfully created a calendar widget";
        ui->topHorizontalLayout->insertWidget(0, cal, 0, Qt::AlignLeft);
        connect(this, &MirrorFrame::dayChanged, cal, &calendar::Calendar::changeDay);
    }
}

void MirrorFrame::updateClock()
{
    const QDate day = ui->clock->date();
    const QDateTime now = QDateTime::currentDateTime();
    const QDate today = now.date();

    ui->clock->setDateTime(now);
    if (today > day) {
        emit dayChanged(today);
    }
}
