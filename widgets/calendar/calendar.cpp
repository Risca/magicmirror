#include "calendar.h"
#include "ui_calendar.h"

#include "data_sources/event.hpp"
#include "data_sources/cal_isource.h"

#include "utils/effects.h"
#include "utils/settingsfactory.h"

#include <QApplication>
#include <QBrush>
#include <QDebug>
#include <QFontMetrics>
#include <QIcon>
#include <QGraphicsOpacityEffect>
#include <QLinearGradient>
#include <QLocale>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QTextCharFormat>
#include <QUrl>

namespace calendar {

namespace {

class EventItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    EventItemDelegate(QObject *parent) : QStyledItemDelegate(parent) { }

    void paint(QPainter *painter, const QStyleOptionViewItem &vopt, const QModelIndex &index) const
    {
#if QT_VERSION >= 0x050E00
        QStringList texts = index.data().toString().split(' ', Qt::SkipEmptyParts);
#else
        QStringList texts = index.data().toString().split(' ', QString::SkipEmptyParts);
#endif
        if (texts.size() < 2) {
            painter->restore();
            return QStyledItemDelegate::paint(painter, vopt, index);
        }
        const QString &date = texts.takeFirst();
        const QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
        const QString summary = texts.join(' ');

        const QWidget *widget = vopt.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();

        painter->save();
        painter->setClipRect(vopt.rect);

        // Draw the background
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &vopt, painter, widget);

        // Draw the date
        painter->save();
        // Reduce the font a bit
        QFont font = painter->font();
        font.setPointSize(font.pointSize() * 2 / 3);
        painter->setFont(font);
        // Write date with darker colors
        painter->setPen(vopt.palette.color(QPalette::Disabled, QPalette::Text));
        QFontMetrics fm = painter->fontMetrics();
#if QT_VERSION >= 0x050B00
        int width = fm.horizontalAdvance(date);
#else
        int width = fm.width(date);
#endif
        QRect textRect = QRect(vopt.rect.topLeft(), QSize(width, vopt.rect.height()));
        painter->drawText(textRect, vopt.displayAlignment, date);
        painter->restore();

        // Draw the icon
        QRect iconRect = QRect(textRect.topRight(), QSize(1, 1) * vopt.rect.height());
        icon.paint(painter, iconRect, Qt::AlignHCenter | Qt::AlignVCenter);

        // Draw the summary
        fm = painter->fontMetrics();
        textRect = QRect(iconRect.topRight(), vopt.rect.bottomRight() + QPoint(1, 1));
        painter->drawText(textRect, vopt.displayAlignment, summary);

        painter->restore();
    }
};

}

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
    ui->events->setItemDelegate(new EventItemDelegate(ui->events));

    ui->calendar->setFirstDayOfWeek(locale().firstDayOfWeek());

    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setInterval(CALENDAR_SYNC_PERIOD);
    connect(&m_timer, SIGNAL(timeout()), m_source, SLOT(sync()));

    connect(m_source, SIGNAL(finished(const QList<Event>&)),
            this, SLOT(NewEventList(const QList<Event>&)));
    connect(m_source, SIGNAL(finished(const QList<Event>&)),
            ui->calendar, SLOT(setEvents(const QList<Event>&)));
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
        qDebug() << locale().toString(e.start, QLocale::LongFormat)
                 << locale().toString(e.stop, QLocale::LongFormat)
                 << e.summary;

        const QString text = locale().toString(e.start, QLocale::ShortFormat) + " " + e.summary;

        QFontMetrics fm = ui->events->fontMetrics();
        // Make icon half as big as the available space
        QPixmap pix(fm.height() / 2, fm.height() / 2);
        pix.fill(Qt::transparent);

        QPainter *p = new QPainter(&pix);
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(Qt::NoPen);
        p->setBrush(QBrush(e.color));
        p->drawEllipse(pix.rect());
        delete p;

        ui->events->addItem(new QListWidgetItem(QIcon(pix), text));
    }

    m_timer.start();
}

}

#include "calendar.moc"
