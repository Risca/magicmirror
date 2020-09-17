#include "MirrorFrame.h"
#include "ui_MirrorFrame.h"

#include "calendar.h"
#include "forecast.h"
#include "currentconditions.h"
#include "settingsfactory.h"

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

    ui->clock->setDisplayFormat(QLocale().timeFormat());
    connect(&m_clockTimer, SIGNAL(timeout()), this, SLOT(updateClock()));
    m_clockTimer.start(500);

    createWeatherSystem();
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

void MirrorFrame::createWeatherSystem()
{
    QVBoxLayout* layout = new QVBoxLayout;

    weather::CurrentConditions* weather = new weather::CurrentConditions(m_net, this);
    layout->addWidget(weather);

    weather::Forecast* forecast = new weather::Forecast(m_net, this);
    layout->addWidget(forecast);

    ui->bottomHorizontalLayout->insertLayout(0, layout);
    ui->bottomHorizontalLayout->setStretch(1,1);
}

void MirrorFrame::createCalendarSystem()
{
    calendar::Calendar* cal;
    if (calendar::Calendar::Create(cal, m_net, this)) {
        qDebug() << "Successfully created a calendar widget";
        ui->topHorizontalLayout->insertWidget(0,cal);
        connect(this, SIGNAL(dayChanged(const QDate&)), cal, SLOT(changeDay(const QDate&)));
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
