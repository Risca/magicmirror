#include "calendar.h"
#include "ui_calendar.h"

#include "data_sources/event.hpp"
#include "data_sources/isource.hpp"

#include "settingsfactory.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QLinearGradient>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QTextCharFormat>
#include <QUrl>

namespace calendar {

namespace {

void SetFadeoutEffect(QWidget* w)
{
    qDebug() << "Applying fade:" << w->rect();
    QLinearGradient alphaGradient(w->rect().topLeft(), w->rect().bottomLeft());
    alphaGradient.setColorAt(0.0, Qt::black);
    alphaGradient.setColorAt(1.0, Qt::transparent);
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
    effect->setOpacityMask(alphaGradient);
    w->setGraphicsEffect(effect);
}

} // anonymous namespace

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

    SetFadeoutEffect(ui->events);

    ui->calendar->setFirstDayOfWeek(m_locale.firstDayOfWeek());

    connect(m_source, SIGNAL(finished(const QList<calendar::Event>&)),
            this, SLOT(NewEventList(const QList<calendar::Event>&)));
    m_source->sync();
}

Calendar::~Calendar()
{
    delete ui;
}

void Calendar::resizeEvent(QResizeEvent *event)
{
    SetFadeoutEffect(ui->events);
    QWidget::resizeEvent(event);
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

        // Make date bold
        QTextCharFormat format = ui->calendar->dateTextFormat(e.start);
        QFont font = this->font();
        font.setBold(true);
        font.setPointSize(font.pointSize() + 1);
        format.setFont(font);
        ui->calendar->setDateTextFormat(e.start, format);

        const QString event = m_locale.toString(e.start, QLocale::ShortFormat) + " " + e.summary;
        ui->events->addItem(event);
    }
    SetFadeoutEffect(ui->events);
}

}
