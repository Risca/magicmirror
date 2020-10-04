#include "calendar.h"
#include "ui_calendar.h"

#include "data_sources/event.hpp"
#include "data_sources/cal_isource.h"

#include "utils/effects.h"
#include "utils/settingsfactory.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QLinearGradient>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QTextCharFormat>
#include <QUrl>

namespace calendar {

#define CALENDAR_SYNC_PERIOD (2 * 60 * 60 * 1000)

bool Calendar::Create(Calendar *&cal, QSharedPointer<QNetworkAccessManager> net, QWidget *parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Calendar");

    ISource* source;
    if (ISource::Create(source, settings, net, parent)) {
        cal = new Calendar(source, parent);
        if (!cal) {
            delete source;
        }
    }
    return (cal == NULL ? false : true);
}

Calendar::Calendar(ISource *dataSource, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calendar),
      m_source(dataSource)
{
    ui->setupUi(this);

    utils::ApplyFade(ui->events);

    ui->calendar->setFirstDayOfWeek(m_locale.firstDayOfWeek());

    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setInterval(CALENDAR_SYNC_PERIOD);
    connect(&m_timer, SIGNAL(timeout()), m_source, SLOT(sync()));

    connect(m_source, SIGNAL(finished(const QList<Event>&)),
            this, SLOT(NewEventList(const QList<Event>&)));
    m_source->sync();

    m_timer.start();
}

Calendar::~Calendar()
{
    delete ui;
}

void Calendar::changeDay(const QDate &day)
{
    ui->month_and_year->setDate(day);
}

void Calendar::NewEventList(const QList<Event> &events)
{
    ui->events->clear();
    ui->calendar->setDateTextFormat(QDate(), QTextCharFormat());
    foreach (const calendar::Event& e, events) {
        qDebug() << m_locale.toString(e.start, QLocale::LongFormat)
                 << m_locale.toString(e.stop, QLocale::LongFormat)
                 << "All day event:" << (e.allDayEvent ? "yes" : "no")
                 << e.summary;

        // Make date(s) bold
        QFont font = this->font();
        font.setBold(true);
        font.setPointSize(font.pointSize() + 1);
        for (QDate date = e.start; date <= e.stop; date = date.addDays(1)) {
            QTextCharFormat format = ui->calendar->dateTextFormat(date);
            format.setFont(font);
            ui->calendar->setDateTextFormat(date, format);
        }

        const QString event = m_locale.toString(e.start, QLocale::ShortFormat) + " " + e.summary;
        ui->events->addItem(event);
    }

    m_timer.start();
}

}
